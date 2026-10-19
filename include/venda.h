#ifndef VENDA_H
#define VENDA_H

#include "produto.h"

/* ==========================================================
   venda.h
   Modulo de vendas: uma venda pertence a um cliente e pode
   conter varios produtos (itens). Persistencia em data/vendas.dat.
   ========================================================== */

#define MAX_ITENS_VENDA 20
#define TAM_DATA        11      /* dd/mm/aaaa + '\0' */

/* Cada item guarda uma "fotografia" do nome e do preco no momento da
   venda, para o historico nao mudar se o produto for alterado depois. */
typedef struct {
    int   codigoProduto;
    char  nomeProduto[TAM_NOME_PROD];
    int   quantidade;
    float precoUnitario;
    float subtotal;
} ItemVenda;

typedef struct {
    int       codigo;
    int       codigoCliente;
    char      data[TAM_DATA];
    int       numItens;
    ItemVenda itens[MAX_ITENS_VENDA];
    float     valorTotal;
} Venda;

void menuVendas(void);
void registrarVenda(void);
void listarVendas(void);
void buscarVenda(void);

/* Carrega todas as vendas (vetor malloc). NULL = erro. */
Venda *carregarVendas(int *total);

/* RECURSAO: soma dos subtotais dos n primeiros itens. */
float totalItensRecursivo(const ItemVenda *itens, int n);

/* Integridade referencial: 1 se existe venda associada. */
int clienteTemVendas(int codigoCliente);
int produtoTemVendas(int codigoProduto);

#endif
