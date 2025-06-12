Trabalho Prático de Algoritmos e Teoria dos Grafos
João Meyer e Vitor Faria - BCC UFPR

Este trabalho implementa um sistema em C para leitura e análise de grafos não-direcionados ponderados.
O sistema permite identificar propriedades importantes do grafo, como:

- Número de vértices e arestas
- Número de componentes conexos
- Verificação se o grafo é bipartido
- Cálculo dos diâmetros dos componentes
- Identificação de vértices de corte (articulação)
- Identificação de arestas de corte (pontes)

O grafo é lido da entrada padrão (stdin), contendo vértices e arestas com pesos opcionais.

- grafo.c  => Implementação das estruturas de dados e algoritmos relacionados ao grafo.
- grafo.h  => Cabeçalho com as definições das funções usada por teste.c.
- teste.c  => Programa principal que utiliza a API para carregar o grafo da entrada padrão e imprimir os resultados.

É necessário ter o compilador gcc instalado e usar make.

EXEMPLO DE EXECUÇÃO:
Comando:

    ./analisador < teste.txt

Conteúdo do arquivo 'teste.txt':

    MeuGrafoDeTeste
    A
    B
    C
    A--B 3
    B--C 4
    C--A 5
    D
    E
    D--E

Saída esperada:

    grafo: MeuGrafoDeTeste
    5 vertices
    4 arestas
    2 componentes
    não bipartido
    diâmetros: 1 5
    vértices de corte:
    arestas de corte: D E