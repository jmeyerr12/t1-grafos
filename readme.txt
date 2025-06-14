Trabalho Prático – Algoritmos e Teoria dos Grafos
Autores: João Meyer e Vitor Faria – BCC/UFPR

Descrição Geral
---------------
Este trabalho implementa, em linguagem C, um sistema para leitura e análise de grafos não-direcionados ponderados, com foco na aplicação de algoritmos clássicos da Teoria dos Grafos.

O sistema lê um grafo da entrada padrão (stdin) e permite:

- Contar o número de vértices e arestas
- Determinar o número de componentes conexos
- Verificar se o grafo é bipartido
- Calcular os diâmetros de cada componente
- Listar os vértices de corte (articulações)
- Listar as arestas de corte (pontes)

O grafo é representado com listas de adjacência e mapeamento de nomes de vértices.

Arquivos
--------
- grafo.c  => Implementação das funções definidas no cabeçalho.
- grafo.h  => Interface pública fornecida, com protótipos e estrutura do grafo.
- teste.c  => Programa principal fornecido que usa a API definida em grafo.h.

Compilação
----------
Use `make` para compilar o projeto. O executável será gerado como `analisador`.

    make

Execução
--------
    ./analisador < teste.txt

Exemplo de Entrada (teste.txt)
------------------------------
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

Saída Esperada
--------------
grafo: MeuGrafoDeTeste
5 vertices
4 arestas
2 componentes
não bipartido
diâmetros: 1 5
vértices de corte:
arestas de corte: D E

Detalhes da Implementação (grafo.c)
-----------------------------------
Representação:
- O grafo é representado como uma lista de adjacência, com vértices mapeados a índices internos.
- Arestas podem ter pesos inteiros não negativos. Caso o peso não seja informado, assume-se peso 1.

Componentes principais:

contar_componentes()
- Utiliza busca em profundidade (DFS) para percorrer o grafo e contar os componentes conexos.

verificar_bipartido()
- Aplica uma busca em largura (BFS) com coloração alternada de vértices.
- Se dois vértices vizinhos recebem a mesma cor, o grafo não é bipartido.

calcular_diametros()
- Usa o algoritmo de Dijkstra para encontrar o diâmetro de cada componente conexo.
- Mesmo que o grafo seja não ponderado, o algoritmo trata arestas sem peso como de peso 1.

vertices_corte()
- Implementa o algoritmo de Tarjan para encontrar vértices de articulação (corte) por meio de low-link values.
- Os resultados são armazenados como strings com os nomes dos vértices em ordem alfabética.

arestas_corte()
- Também baseado em Tarjan, identifica arestas cuja remoção aumenta o número de componentes (pontes).
- As arestas são listadas como pares de vértices, ordenados alfabeticamente e apresentados em ordem crescente.

Funções auxiliares:
- dfs_cortes(): versão iterativa da DFS que preenche estruturas auxiliares para corte.
- cmp_str(): função auxiliar de comparação usada com qsort para ordenar nomes alfabeticamente.

Observação sobre Tarjan:
-------------------------
Para as funções de corte utilizamos o algoritmo de Tarjan. Embora o algoritmo de Tarjan propriamente dito não tenha sido apresentado em aula, sua base está diretamente relacionada aos conceitos estudados, como busca em profundidade (DFS), tempos de descoberta (disc) e os menores tempos acessíveis (low). O algoritmo de Tarjan estende esses conceitos para identificar vértices de articulação (cujos filhos na DFS não conseguem se reconectar com ancestrais sem passar por ele) e arestas de corte (que desconectam componentes se forem removidas). A lógica central consiste em rastrear os caminhos de retorno possíveis durante a DFS, marcando pontos onde a ausência desses caminhos revela dependências estruturais críticas no grafo.