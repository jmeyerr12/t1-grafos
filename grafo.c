#include "grafo.h"
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

// -----------------------------------------------------
// estruturas internas

typedef struct vizinho {
    struct vertice *vert;
    int             peso;
    struct vizinho *prox;
} vizinho;

typedef struct vertice {
    char           *nome;
    vizinho        *adj;
    struct vertice *next;
    unsigned int    id;
} vertice;

struct grafo {
    char        *nome;
    vertice     *vertices;   // lista encadeada
    vertice    **vetor;      // acesso O(1) por id
    unsigned int n_vertices;
    unsigned int n_arestas;
};

typedef struct par_t {
    unsigned int a, b;
} par_t;

// -----------------------------------------------------
// utilidades

static char *dup_string(const char *s) {
    size_t n = strlen(s) + 1;
    char *d = malloc(n);
    if (d) memcpy(d, s, n);
    return d;
}

static vertice *busca_vertice(grafo *g, const char *nome) {
    for (vertice *v = g->vertices; v; v = v->next)
        if (!strcmp(v->nome, nome)) return v;

    vertice *v = calloc(1, sizeof(vertice));
    v->nome = dup_string(nome);
    v->id   = g->n_vertices;

    v->next     = g->vertices;
    g->vertices = v;

    g->vetor = realloc(g->vetor, (g->n_vertices + 1) * sizeof(vertice *));
    g->vetor[g->n_vertices++] = v;
    return v;
}

static void adiciona_aresta(grafo *g, vertice *u, vertice *v, int peso) {
    vizinho *a = calloc(1, sizeof(vizinho));
    a->vert = v;
    a->peso = peso;
    a->prox = u->adj;
    u->adj = a;

    vizinho *b = calloc(1, sizeof(vizinho));
    b->vert = u;
    b->peso = peso;
    b->prox = v->adj;
    v->adj = b;

    g->n_arestas++;
}

static int proxima_linha(FILE *f, char *buf, size_t tam) {
    while (fgets(buf, (int)tam, f)) {
        char *p = buf;
        while (isspace((unsigned char)*p)) p++;
        if (!*p || *p == '\n' || (p[0] == '/' && p[1] == '/')) continue;
        size_t len = strlen(buf);
        if (len && buf[len - 1] == '\n') buf[len - 1] = '\0';
        return 1;
    }
    return 0;
}

// -----------------------------------------------------
// principal

grafo *le_grafo(FILE *f) {
    if (!f) return NULL;
    grafo *g = calloc(1, sizeof(grafo));

    char linha[2048];
    if (!proxima_linha(f, linha, sizeof(linha))) { free(g); return NULL; }
    g->nome = dup_string(linha);

    while (proxima_linha(f, linha, sizeof(linha))) {
        char *seta = strstr(linha, "--");
        if (seta) {
            *seta = *(seta + 1) = ' ';
            char v1[256], v2[256];
            int peso = 0;
            if (sscanf(linha, "%255s %255s %d", v1, v2, &peso) < 2) continue;
            if (peso == 0) peso = 1;
            vertice *a = busca_vertice(g, v1);
            vertice *b = busca_vertice(g, v2);
            if (a && b) adiciona_aresta(g, a, b, peso);
        } else {
            busca_vertice(g, linha);
        }
    }
    return g;
}

unsigned int destroi_grafo(grafo *g) {
    if (!g) return 0;
    for (vertice *v = g->vertices; v; ) {
        for (vizinho *adj = v->adj; adj; ) {
            vizinho *tmp = adj;
            adj = adj->prox;
            free(tmp);
        }
        free(v->nome);
        vertice *tmpv = v;
        v = v->next;
        free(tmpv);
    }
    free(g->vetor);
    free(g->nome);
    free(g);
    return 1;
}

char *nome(grafo *g)               { return g ? g->nome       : NULL; }
unsigned int n_vertices(grafo *g)  { return g ? g->n_vertices : 0;    }
unsigned int n_arestas(grafo *g)   { return g ? g->n_arestas  : 0;    }

// -----------------------------------------------------
// componentes conexos

static void bfs_componentes(grafo *g, int *comp, unsigned int *n_comp) {
    unsigned int n = g->n_vertices;
    for (unsigned int i = 0; i < n; i++) comp[i] = -1;

    int *fila = malloc(n * sizeof(int));
    *n_comp = 0;

    for (unsigned int s = 0; s < n; s++)
        if (comp[s] == -1) {
            comp[s] = (int)(*n_comp);
            int ini = 0, fim = 0;
            fila[fim++] = (int)s;

            while (ini < fim) {
                int u = fila[ini++];
                for (vizinho *adj = g->vetor[u]->adj; adj; adj = adj->prox) {
                    unsigned int v = adj->vert->id;
                    if (comp[v] == -1) {
                        comp[v] = (int)(*n_comp);
                        fila[fim++] = (int)v;
                    }
                }
            }
            (*n_comp)++;
        }
    free(fila);
}

unsigned int n_componentes(grafo *g) {
    if (!g) return 0;
    int *comp = malloc(g->n_vertices * sizeof(int));
    unsigned int ncmp; 
    bfs_componentes(g, comp, &ncmp);
    free(comp);
    return ncmp;
}

// -----------------------------------------------------
// verificação de bipartição

unsigned int bipartido(grafo *g) {
    if (!g) return 0;
    unsigned int n = g->n_vertices;
    char *cor = malloc(n);
    for (unsigned int i = 0; i < n; i++) cor[i] = -1;
    int *fila = malloc(n * sizeof(int));

    for (unsigned int s = 0; s < n; s++)
        if (cor[s] == -1) {
            cor[s] = 0; 
            int ini = 0, fim = 0;
            fila[fim++] = (int)s;

            while (ini < fim) {
                int u = fila[ini++];
                for (vizinho *adj = g->vetor[u]->adj; adj; adj = adj->prox) {
                    unsigned int v = adj->vert->id;
                    if (cor[v] == -1) {
                        cor[v] = 1 - cor[u];
                        fila[fim++] = (int)v;
                    } else if (cor[v] == cor[u]) {
                        free(cor);
                        free(fila);
                        return 0;
                    }
                }
            }
        }
    free(cor);
    free(fila);
    return 1;
}

static void dijkstra_dist(grafo *g, unsigned int s, int *d) {
    unsigned int n = g->n_vertices;

    int *estado = calloc(n, sizeof(int));   // 0: não visitado, 1: na fila, 2: processado
    int *pai = calloc(n, sizeof(int));      // opcional: se quiser rastrear caminhos

    for (unsigned int i = 0; i < n; i++) {
        d[i] = -1;
        pai[i] = -1;
    }

    d[s] = 0;
    estado[s] = 1;

    int *fila = malloc(n * sizeof(int));
    int tam = 0;
    fila[tam++] = (int)s;

    while (tam > 0) {
        // encontra vértice com menor custo na fila
        int idx_min = 0;
        for (int i = 1; i < tam; i++) {
            if (d[fila[i]] < d[fila[idx_min]]) {
                idx_min = i;
            }
        }

        int v = fila[idx_min];
        // remove v da fila
        fila[idx_min] = fila[--tam];

        for (vizinho *adj = g->vetor[v]->adj; adj; adj = adj->prox) {
            unsigned int u = adj->vert->id;
            int custo = d[v] + adj->peso;

            if (estado[u] == 1) {
                if (custo < d[u]) {
                    d[u] = custo;
                    pai[u] = v;
                }
            } else if (estado[u] == 0) {
                d[u] = custo;
                pai[u] = v;
                fila[tam++] = (int)u;
                estado[u] = 1;
            }
        }

        estado[v] = 2;
    }

    free(fila);
    free(pai);     // se não for usar caminhos
    free(estado);
}

// -----------------------------------------------------
// diametros dos componentes

char *diametros(grafo *g) {
    if (!g) return NULL;
    unsigned int n = g->n_vertices;
    if (!n) return dup_string("");

    int *comp = malloc(n * sizeof(int));
    unsigned int ncmp; 
    bfs_componentes(g, comp, &ncmp);

    int *diam = calloc(ncmp, sizeof(int));
    int *dist = malloc(n * sizeof(int));

    for (unsigned int v = 0; v < n; v++) {
        dijkstra_dist(g, v, dist);
        int ecc = 0;
        for (unsigned int u = 0; u < n; u++)
            if (comp[u] == comp[v] && dist[u] != -1 && dist[u] > ecc)
                ecc = dist[u];
        if (ecc > diam[comp[v]]) diam[comp[v]] = ecc;
    }

    for (unsigned int i = 0; i < ncmp; i++)
        for (unsigned int j = i + 1; j < ncmp; j++)
            if (diam[i] > diam[j]) {
                int t = diam[i];
                diam[i] = diam[j];
                diam[j] = t;
            }

    size_t tam = ncmp * 12;
    char *resp = malloc(tam);
    resp[0] = '\0';
    for (unsigned int i = 0; i < ncmp; i++) {
        char buf[32];
        sprintf(buf, "%d", diam[i]);
        if (i) strcat(resp, " ");
        strcat(resp, buf);
    }

    free(comp);
    free(diam);
    free(dist);
    return resp;
}

// -----------------------------------------------------
// dfs para vértices/arestas de corte

static void dfs_cortes_rec(
    grafo *g, unsigned int u,
    int *visit, int *disc, int *low, int *parent,
    char *art, unsigned int *n_art,
    par_t *brid, unsigned int *n_br,
    int *tempo)
{
    visit[u] = 1;
    disc[u] = low[u] = ++(*tempo);
    int filhos = 0;

    for (vizinho *adj = g->vetor[u]->adj; adj; adj = adj->prox) {
        unsigned int v = adj->vert->id;
        if (!visit[v]) {
            parent[v] = (int)u;
            filhos++;
            dfs_cortes_rec(g, v, visit, disc, low, parent, art, n_art, brid, n_br, tempo);
            if (low[v] < low[u]) low[u] = low[v];

            if ((parent[u] == -1 && filhos > 1) ||
                (parent[u] != -1 && low[v] >= disc[u]))
                if (!art[u]) { art[u] = 1; (*n_art)++; }

            if (low[v] > disc[u]) {
                brid[*n_br].a = u;
                brid[*n_br].b = v;
                (*n_br)++;
            }
        } else if (v != (unsigned int)parent[u] && disc[v] < low[u]) {
            low[u] = disc[v];
        }
    }
}

void dfs_cortes(
    grafo *g, unsigned int u,
    int *visit, int *disc, int *low, int *parent,
    char *art, unsigned int *n_art,
    par_t *brid, unsigned int *n_br)
{
    int tempo = 0;
    dfs_cortes_rec(g, u, visit, disc, low, parent, art, n_art, brid, n_br, &tempo);
}

int cmp_str(const void *a, const void *b) {
    const char * const *sa = (const char * const *)a;
    const char * const *sb = (const char * const *)b;
    return strcmp(*sa, *sb);
}

char *vertices_corte(grafo *g) {
    unsigned int n = g->n_vertices;

    int *visit = calloc(n, sizeof(int));
    int *disc = calloc(n, sizeof(int));
    int *low = calloc(n, sizeof(int));
    int *parent = malloc(n * sizeof(int));
    char *art = calloc(n, sizeof(char));  // vertices de corte

    for (unsigned int i = 0; i < n; i++) parent[i] = -1;

    unsigned int n_art = 0;
    par_t *brid = malloc(n * sizeof(*brid));
    unsigned int n_br = 0;

    for (unsigned int i = 0; i < n; i++) {
        if (!visit[i]) {
            dfs_cortes(g, i, visit, disc, low, parent, art, &n_art, brid, &n_br);
        }
    }

    if (n_art == 0) {
        free(visit); free(disc); free(low); free(parent); free(art); free(brid);
        char *res = malloc(1);
        res[0] = '\0';
        return res;
    }

    const char **nomes = malloc(n_art * sizeof(char *));
    unsigned int idx = 0;
    for (unsigned int i = 0; i < n; i++) {
        if (art[i]) {
            nomes[idx++] = g->vetor[i]->nome;
        }
    }

    qsort(nomes, n_art, sizeof(char *), cmp_str);

    size_t tamanho = 0;
    for (unsigned int i = 0; i < n_art; i++) {
        tamanho += strlen(nomes[i]) + 1; // espaço ou \0
    }

    char *resultado = malloc(tamanho);
    resultado[0] = '\0';

    for (unsigned int i = 0; i < n_art; i++) {
        strcat(resultado, nomes[i]);
        if (i < n_art - 1) strcat(resultado, " ");
    }

    free(visit);
    free(disc);
    free(low);
    free(parent);
    free(art);
    free(brid);
    free(nomes);

    return resultado;
}

char *arestas_corte(grafo *g) {
    if (!g) return NULL;
    unsigned int n = g->n_vertices;
    if (!n) return dup_string("");

    int *visit  = calloc(n, sizeof(int));
    int *disc   = calloc(n, sizeof(int));
    int *low    = calloc(n, sizeof(int));
    int *parent = malloc(n * sizeof(int));
    char *art   = calloc(n, sizeof(char));
    for (unsigned int i = 0; i < n; i++) parent[i] = -1;

    par_t *bridges = malloc(g->n_arestas * sizeof(*bridges));
    unsigned int n_br = 0, n_art = 0;

    for (unsigned int i = 0; i < n; i++)
        if (!visit[i])
            dfs_cortes(g, i, visit, disc, low, parent, art, &n_art, bridges, &n_br);

    if (!n_br) {
        free(visit); free(disc); free(low); free(parent); free(art); free(bridges);
        return dup_string("");
    }

    struct par { char *a, *b; } *pares = malloc(n_br * sizeof(*pares));
    for (unsigned int i = 0; i < n_br; i++) {
        char *x = g->vetor[bridges[i].a]->nome;
        char *y = g->vetor[bridges[i].b]->nome;
        if (strcmp(x, y) < 0) { pares[i].a = x; pares[i].b = y; }
        else                   { pares[i].a = y; pares[i].b = x; }
    }

    for (unsigned int i = 0; i < n_br; i++)
        for (unsigned int j = i + 1; j < n_br; j++) {
            int c = strcmp(pares[i].a, pares[j].a);
            if (c > 0 || (c == 0 && strcmp(pares[i].b, pares[j].b) > 0)) {
                struct par t = pares[i];
                pares[i] = pares[j];
                pares[j] = t;
            }
        }

    size_t tam = 1;
    for (unsigned int i = 0; i < n_br; i++)
        tam += strlen(pares[i].a) + strlen(pares[i].b) + 2;
    char *resp = malloc(tam);
    resp[0] = '\0';
    for (unsigned int i = 0; i < n_br; i++) {
        if (i) strcat(resp, " ");
        strcat(resp, pares[i].a);
        strcat(resp, " ");
        strcat(resp, pares[i].b);
    }

    free(visit); free(disc); free(low); free(parent); free(art); free(bridges); free(pares);
    return resp;
}
