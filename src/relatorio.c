#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/relatorio.h"
#include "../include/cliente.h"
#include "../include/produto.h"
#include "../include/venda.h"
#include "../include/utils.h"

/* ==========================================================
   Blocos de relatorio (NAO limpam a tela: podem ser combinados)
   ========================================================== */

/* Acumulador usado para descobrir o produto mais vendido. */
typedef struct {
    int    codigo;
    char   nome[TAM_NOME_PROD];
    long   quantidade;
    double valor;
} AcumuladoProduto;

/* Quantidades, valor do estoque e totais de vendas. */
static void imprimirResumo(void) {
    int nClientes, nProdutos, nVendas;
    Cliente *clientes = carregarClientes(&nClientes);
    Produto *produtos = carregarProdutos(&nProdutos);
    Venda   *vendas   = carregarVendas(&nVendas);

    if (clientes == NULL || produtos == NULL || vendas == NULL) {
        printf(COR_VERMELHO "Erro ao ler os arquivos de dados.\n" COR_RESET);
    } else {
        double totalVendido = 0.0;
        for (int i = 0; i < nVendas; i++) {
            totalVendido += vendas[i].valorTotal;
        }
        printf("Quantidade total de clientes cadastrados : " COR_NEGRITO "%d\n" COR_RESET, nClientes);
        printf("Quantidade total de produtos cadastrados : " COR_NEGRITO "%d\n" COR_RESET, nProdutos);
        printf("Valor total estimado do estoque          : " COR_NEGRITO "R$ %.2f\n" COR_RESET,
               valorEstoqueRecursivo(produtos, nProdutos));
        printf("Quantidade de vendas realizadas          : " COR_NEGRITO "%d\n" COR_RESET, nVendas);
        printf("Valor total vendido                      : " COR_NEGRITO "R$ %.2f\n" COR_RESET,
               totalVendido);
    }
    free(clientes);
    free(produtos);
    free(vendas);
}

static void imprimirProdutoMaisVendido(void) {
    int nVendas, nProdutos;
    Venda   *vendas   = carregarVendas(&nVendas);
    Produto *produtos = carregarProdutos(&nProdutos);

    if (vendas == NULL || produtos == NULL) {
        printf(COR_VERMELHO "Erro ao ler os arquivos de dados.\n" COR_RESET);
        free(vendas);
        free(produtos);
        return;
    }

    int totalItens = 0;
    for (int i = 0; i < nVendas; i++) totalItens += vendas[i].numItens;

    if (totalItens == 0) {
        printf(COR_AMARELO "Produto mais vendido: nenhuma venda registrada ainda.\n" COR_RESET);
        free(vendas);
        free(produtos);
        return;
    }

    /* Soma as quantidades por produto (um acumulador por codigo). */
    AcumuladoProduto *acc = (AcumuladoProduto *)calloc((size_t)totalItens, sizeof(AcumuladoProduto));
    if (acc == NULL) {
        printf(COR_VERMELHO "Memoria insuficiente.\n" COR_RESET);
        free(vendas);
        free(produtos);
        return;
    }
    int nAcc = 0;
    for (int i = 0; i < nVendas; i++) {
        for (int j = 0; j < vendas[i].numItens; j++) {
            const ItemVenda *it = &vendas[i].itens[j];
            int k;
            for (k = 0; k < nAcc; k++) {
                if (acc[k].codigo == it->codigoProduto) break;
            }
            if (k == nAcc) {
                acc[k].codigo = it->codigoProduto;
                snprintf(acc[k].nome, TAM_NOME_PROD, "%s", it->nomeProduto);
                nAcc++;
            }
            acc[k].quantidade += it->quantidade;
            acc[k].valor += it->subtotal;
        }
    }

    int melhor = 0;
    for (int k = 1; k < nAcc; k++) {
        if (acc[k].quantidade > acc[melhor].quantidade) melhor = k;
    }

    /* Mostra o nome ATUAL do produto, se ele ainda existir. */
    const char *nome = acc[melhor].nome;
    int idx = buscaBinariaProduto(produtos, 0, nProdutos - 1, acc[melhor].codigo);
    if (idx >= 0) nome = produtos[idx].nome;

    printf("Produto mais vendido: " COR_NEGRITO "%s" COR_RESET " (cod %d)\n",
           nome, acc[melhor].codigo);
    printf("  Unidades vendidas: %ld | Faturamento: R$ %.2f\n",
           acc[melhor].quantidade, acc[melhor].valor);

    free(acc);
    free(vendas);
    free(produtos);
}

static void imprimirClienteMaiorVolume(void) {
    int nVendas, nClientes;
    Venda   *vendas   = carregarVendas(&nVendas);
    Cliente *clientes = carregarClientes(&nClientes);

    if (vendas == NULL || clientes == NULL) {
        printf(COR_VERMELHO "Erro ao ler os arquivos de dados.\n" COR_RESET);
        free(vendas);
        free(clientes);
        return;
    }
    if (nVendas == 0 || nClientes == 0) {
        printf(COR_AMARELO "Cliente com maior volume de compras: nenhuma venda registrada ainda.\n" COR_RESET);
        free(vendas);
        free(clientes);
        return;
    }

    double *soma = (double *)calloc((size_t)nClientes, sizeof(double));
    int    *qtd  = (int *)calloc((size_t)nClientes, sizeof(int));
    if (soma == NULL || qtd == NULL) {
        printf(COR_VERMELHO "Memoria insuficiente.\n" COR_RESET);
    } else {
        for (int i = 0; i < nVendas; i++) {
            int idx = buscaBinariaCliente(clientes, 0, nClientes - 1, vendas[i].codigoCliente);
            if (idx >= 0) {
                soma[idx] += vendas[i].valorTotal;
                qtd[idx]++;
            }
        }
        int melhor = 0;
        for (int i = 1; i < nClientes; i++) {
            if (soma[i] > soma[melhor]) melhor = i;
        }
        printf("Cliente com maior volume de compras: " COR_NEGRITO "%s" COR_RESET " (cod %d)\n",
               clientes[melhor].nome, clientes[melhor].codigo);
        printf("  Compras realizadas: %d | Total comprado: R$ %.2f\n", qtd[melhor], soma[melhor]);
    }
    free(soma);
    free(qtd);
    free(vendas);
    free(clientes);
}

static void imprimirTicketMedio(void) {
    int nVendas;
    Venda *vendas = carregarVendas(&nVendas);
    if (vendas == NULL) {
        printf(COR_VERMELHO "Erro ao ler o arquivo de vendas.\n" COR_RESET);
        return;
    }
    if (nVendas == 0) {
        printf(COR_AMARELO "Nenhuma venda registrada ainda.\n" COR_RESET);
        free(vendas);
        return;
    }
    double soma = 0.0;
    int maior = 0, menor = 0;
    for (int i = 0; i < nVendas; i++) {
        soma += vendas[i].valorTotal;
        if (vendas[i].valorTotal > vendas[maior].valorTotal) maior = i;
        if (vendas[i].valorTotal < vendas[menor].valorTotal) menor = i;
    }
    printf("Ticket medio por venda : " COR_NEGRITO "R$ %.2f\n" COR_RESET, soma / nVendas);
    printf("Maior venda            : venda %d (R$ %.2f)\n", vendas[maior].codigo, vendas[maior].valorTotal);
    printf("Menor venda            : venda %d (R$ %.2f)\n", vendas[menor].codigo, vendas[menor].valorTotal);
    free(vendas);
}

/* ==========================================================
   Telas / menus
   ========================================================== */

static void relatorioGeral(void) {
    imprimirCabecalho("RELATORIO GERAL", COR_AZUL);
    printf("\n");
    imprimirResumo();
    printf("\n");
    imprimirProdutoMaisVendido();
    imprimirClienteMaiorVolume();
    printf("\n" COR_NEGRITO "Produtos com estoque baixo:\n" COR_RESET);
    imprimirProdutosEstoqueBaixo();
}

static void historicoCliente(void) {
    imprimirCabecalho("HISTORICO DE COMPRAS DO CLIENTE", COR_AZUL);

    int nClientes, nVendas;
    Cliente *clientes = carregarClientes(&nClientes);
    Venda   *vendas   = carregarVendas(&nVendas);

    if (clientes == NULL || vendas == NULL) {
        printf(COR_VERMELHO "Erro ao ler os arquivos de dados.\n" COR_RESET);
    } else {
        int cod = lerCodigo("Codigo do cliente (0 cancela): ");
        if (cod != 0) {
            int idx = buscaBinariaCliente(clientes, 0, nClientes - 1, cod);
            if (idx < 0) {
                printf(COR_VERMELHO "\nCliente nao encontrado.\n" COR_RESET);
            } else {
                printf("\nCliente: " COR_NEGRITO "%s" COR_RESET "\n\n", clientes[idx].nome);
                int qtd = 0;
                double soma = 0.0;
                for (int i = 0; i < nVendas; i++) {
                    if (vendas[i].codigoCliente == cod) {
                        printf("Venda %04d | %s | %d item(ns) | R$ %.2f\n", vendas[i].codigo,
                               vendas[i].data, vendas[i].numItens, vendas[i].valorTotal);
                        qtd++;
                        soma += vendas[i].valorTotal;
                    }
                }
                if (qtd == 0) {
                    printf(COR_AMARELO "Este cliente ainda nao realizou compras.\n" COR_RESET);
                } else {
                    printf(COR_VERDE "\n%d compra(s) | Total: R$ %.2f\n" COR_RESET, qtd, soma);
                }
            }
        }
    }
    free(clientes);
    free(vendas);
}

void menuRelatorios(void) {
    int opcao;

    do {
        imprimirCabecalho("CONSULTAS E RELATORIOS", COR_AZUL);
        printf(COR_AZUL "\n  1" COR_RESET " - Relatorio geral (todos os indicadores)\n");
        printf(COR_AZUL "  2" COR_RESET " - Produtos com estoque baixo\n");
        printf(COR_AZUL "  3" COR_RESET " - Historico de compras de um cliente\n");
        printf(COR_VERMELHO "  0" COR_RESET " - Voltar ao menu principal\n");
        imprimirLinha(COR_AZUL);

        opcao = lerInteiro(COR_NEGRITO ">> Escolha uma opcao: " COR_RESET);

        switch (opcao) {
            case 1: relatorioGeral();        pausar(); break;
            case 2: produtosEstoqueBaixo();  pausar(); break;
            case 3: historicoCliente();      pausar(); break;
            case 0:
                printf(COR_VERDE "\nVoltando ao menu principal...\n" COR_RESET);
                break;
            default:
                printf(COR_VERMELHO "\nOpcao invalida! Tente novamente.\n" COR_RESET);
                pausar();
        }
    } while (opcao != 0);
}

void menuEstatisticas(void) {
    int opcao;

    do {
        imprimirCabecalho("ESTATISTICAS", COR_AZUL);
        printf(COR_AZUL "\n  1" COR_RESET " - Produto mais vendido\n");
        printf(COR_AZUL "  2" COR_RESET " - Cliente com maior volume de compras\n");
        printf(COR_AZUL "  3" COR_RESET " - Ticket medio e extremos de venda\n");
        printf(COR_AZUL "  4" COR_RESET " - Totais gerais (clientes, produtos, estoque, vendas)\n");
        printf(COR_VERMELHO "  0" COR_RESET " - Voltar ao menu principal\n");
        imprimirLinha(COR_AZUL);

        opcao = lerInteiro(COR_NEGRITO ">> Escolha uma opcao: " COR_RESET);

        switch (opcao) {
            case 1:
                imprimirCabecalho("PRODUTO MAIS VENDIDO", COR_AZUL);
                printf("\n");
                imprimirProdutoMaisVendido();
                pausar();
                break;
            case 2:
                imprimirCabecalho("CLIENTE COM MAIOR VOLUME DE COMPRAS", COR_AZUL);
                printf("\n");
                imprimirClienteMaiorVolume();
                pausar();
                break;
            case 3:
                imprimirCabecalho("TICKET MEDIO", COR_AZUL);
                printf("\n");
                imprimirTicketMedio();
                pausar();
                break;
            case 4:
                imprimirCabecalho("TOTAIS GERAIS", COR_AZUL);
                printf("\n");
                imprimirResumo();
                pausar();
                break;
            case 0:
                printf(COR_VERDE "\nVoltando ao menu principal...\n" COR_RESET);
                break;
            default:
                printf(COR_VERMELHO "\nOpcao invalida! Tente novamente.\n" COR_RESET);
                pausar();
        }
    } while (opcao != 0);
}
