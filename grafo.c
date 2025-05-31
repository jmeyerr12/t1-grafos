#include "grafo.h"
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

// -----------------------------------------------------
// Estruturas internas

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

// -----------------------------------------------------
// Utilidades

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
// API principal

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
// Componentes conexos

static void bfs_componentes(grafo *g, int *comp, unsigned int *n_comp) {
    unsigned int n = g->n_vertices;
    for (unsigned int i = 0; i < n; i++) comp[i] = -1;

    int *fila = malloc(n * sizeof(int));
    *n_comp = 0;

    for (unsigned int s = 0; s < n; s++)
        if (comp[s] == -1) {
            comp[s] = (*n_comp);
            int ini = 0, fim = 0;
            fila[fim++] = s;

            while (ini < fim) {
                int u = fila[ini++];
                for (vizinho *adj = g->vetor[u]->adj; adj; adj = adj->prox) {
                    unsigned int v = adj->vert->id;
                    if (comp[v] == -1) {
                        comp[v] = *n_comp;
                        fila[fim++] = v;
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
// Verificação de bipartição

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
            fila[fim++] = s;

            while (ini < fim) {
                int u = fila[ini++];
                for (vizinho *adj = g->vetor[u]->adj; adj; adj = adj->prox) {
                    unsigned int v = adj->vert->id;
                    if (cor[v] == -1) {
                        cor[v] = 1 - cor[u];
                        fila[fim++] = v;
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

// -----------------------------------------------------
// Distâncias BFS não ponderadas

static void bfs_dist(grafo *g, unsigned int s, int *d) {
    unsigned int n = g->n_vertices;
    for (unsigned int i = 0; i < n; i++) d[i] = -1;
    int *fila = malloc(n * sizeof(int));

    d[s] = 0;
    int ini = 0, fim = 0;
    fila[fim++] = s;
    while (ini < fim) {
        int u = fila[ini++];
        for (vizinho *adj = g->vetor[u]->adj; adj; adj = adj->prox) {
            unsigned int v = adj->vert->id;
            if (d[v] == -1) {
                d[v] = d[u] + 1;
                fila[fim++] = v;
            }
        }
    }
    free(fila);
}

// -----------------------------------------------------
// Diâmetros dos componentes

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
        bfs_dist(g, v, dist);
        int ecc = 0;
        for (unsigned int u = 0; u < n; u++)
            if (comp[u] == comp[v] && dist[u] > ecc) ecc = dist[u];
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
// DFS para vértices/arestas de corte

static void dfs_cortes(
    grafo *g, unsigned int u,
    int *visit, int *disc, int *low, int *parent,
    char *art, unsigned int *n_art,
    struct { unsigned int a, b; } *brid, unsigned int *n_br)
{
    static int tempo = 0;
    visit[u] = 1;
    disc[u] = low[u] = ++tempo;
    int filhos = 0;

    for (vizinho *adj = g->vetor[u]->adj; adj; adj = adj->prox) {
        unsigned int v = adj->vert->id;
        if (!visit[v]) {
            parent[v] = u;
            filhos++;
            dfs_cortes(g, v, visit, disc, low, parent, art, n_art, brid, n_br);
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

char *vertices_corte(grafo *g) {
    if (!g) return NULL;
    unsigned int n = g->n_vertices;
    if (!n) return dup_string("");

    int *visit  = calloc(n, sizeof(int));
    int *disc   = calloc(n, sizeof(int));
    int *low    = calloc(n, sizeof(int));
    int *parent = malloc(n * sizeof(int));
    char *art   = calloc(n, sizeof(char));
    for (unsigned int i = 0; i < n; i++) parent[i] = -1;

    unsigned int n_art = 0;
    struct { unsigned int a, b; } dummy[1];
    unsigned int nb = 0;

    for (unsigned int i = 0; i < n; i++)
        if (!visit[i])
            dfs_cortes(g, i, visit, disc, low, parent, art, &n_art, dummy, &nb);

    if (!n_art) {
        free(visit); free(disc); free(low); free(parent); free(art);
        return dup_string("");
    }

    char **lista = malloc(n_art * sizeof(char *));
    unsigned int k = 0;
    for (unsigned int i = 0; i < n; i++)
        if (art[i]) lista[k++] = g->vetor[i]->nome;

    for (unsigned int i = 0; i < k; i++)
        for (unsigned int j = i + 1; j < k; j++)
            if (strcmp(lista[i], lista[j]) > 0) {
                char *t = lista[i];
                lista[i] = lista[j];
                lista[j] = t;
            }

    size_t tam = 1;
    for (unsigned int i = 0; i < k; i++) tam += strlen(lista[i]) + 1;
    char *resp = malloc(tam);
    resp[0] = '\0';
    for (unsigned int i = 0; i < k; i++) {
        if (i) strcat(resp, " ");
        strcat(resp, lista[i]);
    }

    free(lista); free(visit); free(disc); free(low); free(parent); free(art);
    return resp;
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

    struct { unsigned int a, b; } *bridges = malloc(g->n_arestas * sizeof(*bridges));
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
