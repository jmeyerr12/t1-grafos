# Trabalho Prático de Algoritmos e Teoria dos Grafos - João Meyer e Vitor Faria - BCC UFPR

## Descrição

Este trabalho implementa um sistema em C para leitura e análise de grafos não-direcionados ponderados. O sistema permite identificar propriedades importantes do grafo, como:

- Número de vértices e arestas
- Número de componentes conexos
- Verificação se o grafo é bipartido
- Cálculo dos diâmetros dos componentes
- Identificação de vértices de corte (articulação)
- Identificação de arestas de corte (pontes)

O grafo é lido de `stdin`, contendo vértices e arestas com pesos opcionais.

---

## Estrutura de Arquivos

- `grafo.c` — Implementação das estruturas de dados e algoritmos relacionados ao grafo.
- `grafo.h` — Cabeçalho com as definições da API pública usada por `teste.c`.
- `teste.c` — Programa principal que utiliza a API para carregar o grafo da entrada padrão e imprimir os resultados.

---

## Compilação
Tenha o compilador C padrão `gcc` e use "make".

## Exemplo de execução

```bash
./analisador < teste.txt
```

### Conteúdo do arquivo `teste.txt`:

```
meuGrafo
A
B
C
D
A -- B 2
B -- C
C -- D
```

### Saída esperada:

```
grafo: meuGrafo
4 vertices
3 arestas
1 componentes
bipartido
diâmetros: 2
vértices de corte: B C
arestas de corte: A B B C C D
```