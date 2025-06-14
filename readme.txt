Trabalho Prático – Algoritmos e Teoria dos Grafos
Autores: João Meyer e Vitor Faria – BCC/UFPR

Descrição Geral
---------------
Este trabalho implementa um sistema em linguagem C para leitura e análise de grafos não-direcionados ponderados, com foco em propriedades estruturais fundamentais da Teoria dos Grafos.

O sistema realiza a leitura do grafo a partir da entrada padrão (stdin), e permite a identificação das seguintes propriedades:

- Número de vértices e arestas
- Número de componentes conexos
- Verificação se o grafo é bipartido
- Cálculo do diâmetro de cada componente conexo
- Identificação de vértices de corte (articulação)
- Identificação de arestas de corte (pontes)

Estrutura de Arquivos
---------------------
- grafo.c  => Implementação das estruturas de dados e dos algoritmos principais do sistema.
- grafo.h  => Cabeçalho fornecido com a definição das funções utilizadas por teste.c.
- teste.c  => Arquivo fornecido com o programa principal que utiliza a API para carregar o grafo e imprimir os resultados.

Compilação
----------
Certifique-se de que o compilador gcc está instalado.
Para compilar o projeto, utilize:

    make

O executável gerado será chamado 'teste'.

Execução
--------
    ./teste < teste.txt

Exemplo de Entrada (teste.in)
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
- O grafo é representado como uma lista de adjacência, usando hash maps para mapear os nomes dos vértices para índices internos.
- As arestas armazenam peso, com valor padrão 1 caso não seja especificado.

Componentes principais:

contar_componentes()
- Usa DFS (busca em profundidade) para identificar componentes conexos.
- Marca os vértices visitados e incrementa o contador de componentes.

verificar_bipartido()
- Aplica DFS com coloração alternada para detectar conflitos de bipartição.
- Se dois vértices vizinhos forem coloridos iguais, o grafo não é bipartido.

calcular_diametros()
- Para cada componente conexo, realiza duas BFS sucessivas para encontrar o maior caminho (diâmetro).
- Usa uma BFS padrão iniciando de um vértice qualquer e depois da extremidade mais distante encontrada.

buscar_articulacoes() e buscar_pontes()
- Baseadas em algoritmo de Tarjan, usando low e discovery time.
- Detectam vértices e arestas cuja remoção aumenta o número de componentes conexos.