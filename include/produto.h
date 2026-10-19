#ifndef PRODUTO_H
#define PRODUTO_H

/* ==========================================================
   produto.h
   Modulo responsavel pelo cadastro de produtos e controle de
   estoque (CRUD completo, com persistencia em data/produtos.dat).
   ========================================================== */

#define TAM_NOME_PROD  60
#define TAM_CATEGORIA  40

/* Um produto com quantidade <= LIMITE_ESTOQUE_BAIXO e considerado
   "estoque baixo". */
#define LIMITE_ESTOQUE_BAIXO 5

typedef struct {
    int   codigo;
    char  nome[TAM_NOME_PROD];
    char  categoria[TAM_CATEGORIA];
    float preco;
    int   quantidadeEstoque;
} Produto;

/* Submenu de Produtos. */
void menuProdutos(void);

/* Operacoes do CRUD e de estoque. */
void cadastrarProduto(void);
void listarProdutos(void);
void buscarProduto(void);
void alterarProduto(void);
void excluirProduto(void);
void consultarEstoque(void);
void produtosEstoqueBaixo(void);

/* ---------- Funcoes de apoio usadas tambem por outros modulos ---------- */

/* Carrega todos os produtos (vetor malloc, ORDENADO por codigo). */
Produto *carregarProdutos(int *total);

/* Ordena por codigo e grava o vetor inteiro no arquivo. 1 = ok. */
int salvarProdutos(Produto *v, int total);

/* RECURSAO: busca binaria recursiva por codigo (vetor ordenado). */
int buscaBinariaProduto(const Produto *v, int ini, int fim, int codigo);

/* RECURSAO: valor total do estoque = soma de preco * quantidade. */
double valorEstoqueRecursivo(const Produto *v, int n);

/* Imprime (sem limpar a tela) a lista de produtos com estoque baixo. */
void imprimirProdutosEstoqueBaixo(void);

#endif
