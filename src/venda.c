#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/venda.h"
#include "../include/cliente.h"
#include "../include/produto.h"
#include "../include/utils.h"
#include "../include/arquivo.h"

/* ==========================================================
   Persistencia e funcoes de apoio
   ========================================================== */

Venda *carregarVendas(int *total) {
    return (Venda *)carregarRegistros(ARQ_VENDAS, sizeof(Venda), total);
}

/* RECURSAO: total = subtotal do ultimo item + total dos n-1 anteriores.
   Caso base: n <= 0 -> 0. */
float totalItensRecursivo(const ItemVenda *itens, int n) {
    if (n <= 0) return 0.0f;
    return itens[n - 1].subtotal + totalItensRecursivo(itens, n - 1);
}

int clienteTemVendas(int codigoCliente) {
    int total;
    Venda *v = carregarVendas(&total);
    if (v == NULL) return 1;     /* na duvida, protege o cliente */
    int tem = 0;
    for (int i = 0; i < total && !tem; i++) {
        if (v[i].codigoCliente == codigoCliente) tem = 1;
    }
    free(v);
    return tem;
}

int produtoTemVendas(int codigoProduto) {
    int total;
    Venda *v = carregarVendas(&total);
    if (v == NULL) return 1;
    int tem = 0;
    for (int i = 0; i < total && !tem; i++) {
        for (int j = 0; j < v[i].numItens; j++) {
            if (v[i].itens[j].codigoProduto == codigoProduto) { tem = 1; break; }
        }
    }
    free(v);
    return tem;
}

static const char *nomeDoCliente(const Cliente *clientes, int n, int codigo) {
    int idx = buscaBinariaCliente(clientes, 0, n - 1, codigo);
    return (idx >= 0) ? clientes[idx].nome : "(cliente removido)";
}

static void exibirVenda(const Venda *v, const char *nomeCliente) {
    imprimirLinha(COR_AZUL);
    printf(COR_AZUL COR_NEGRITO "VENDA N. %04d" COR_RESET "   Data: %s\n", v->codigo, v->data);
    printf("Cliente: %s (cod %d)\n\n", nomeCliente, v->codigoCliente);
    printf(COR_NEGRITO "%-6s %-26s %5s %11s %12s\n" COR_RESET,
           "COD", "PRODUTO", "QTD", "UNITARIO", "SUBTOTAL");
    for (int i = 0; i < v->numItens; i++) {
        const ItemVenda *it = &v->itens[i];
        printf("%-6d %-26.26s %5d R$%9.2f R$%10.2f\n",
               it->codigoProduto, it->nomeProduto, it->quantidade,
               it->precoUnitario, it->subtotal);
    }
    printf(COR_VERDE COR_NEGRITO "\nTOTAL DA VENDA: R$ %.2f\n" COR_RESET, v->valorTotal);
    imprimirLinha(COR_AZUL);
}

/* ==========================================================
   Menu
   ========================================================== */

void menuVendas(void) {
    int opcao;

    do {
        imprimirCabecalho("MODULO DE VENDAS", COR_AZUL);
        printf(COR_AZUL "\n  1" COR_RESET " - Registrar venda\n");
        printf(COR_AZUL "  2" COR_RESET " - Listar vendas\n");
        printf(COR_AZUL "  3" COR_RESET " - Consultar venda por codigo\n");
        printf(COR_VERMELHO "  0" COR_RESET " - Voltar ao menu principal\n");
        imprimirLinha(COR_AZUL);

        opcao = lerInteiro(COR_NEGRITO ">> Escolha uma opcao: " COR_RESET);

        switch (opcao) {
            case 1: registrarVenda(); pausar(); break;
            case 2: listarVendas();   pausar(); break;
            case 3: buscarVenda();    pausar(); break;
            case 0:
                printf(COR_VERDE "\nVoltando ao menu principal...\n" COR_RESET);
                break;
            default:
                printf(COR_VERMELHO "\nOpcao invalida! Tente novamente.\n" COR_RESET);
                pausar();
        }
    } while (opcao != 0);
}

/* ==========================================================
   Registro de venda
   ========================================================== */

/* Monta a venda em memoria. O vetor 'produtos' e alterado (baixa de
   estoque) mas SO e gravado em disco se o usuario confirmar a venda. */
static void processarVenda(const Cliente *clientes, int nClientes,
                           Produto *produtos, int nProdutos, int codigoVenda) {
    Venda venda;
    memset(&venda, 0, sizeof(venda));
    venda.codigo = codigoVenda;
    printf("Venda numero %04d\n\n", codigoVenda);
    avisoCancelar();

    /* 1) Cliente precisa existir. */
    int idxCliente;
    while (1) {
        int cod = lerCodigo("Codigo do cliente (0 cancela): ");
        if (cod == 0) {
            printf(COR_AMARELO "Venda cancelada.\n" COR_RESET);
            return;
        }
        idxCliente = buscaBinariaCliente(clientes, 0, nClientes - 1, cod);
        if (idxCliente >= 0) {
            venda.codigoCliente = cod;
            break;
        }
        printf(COR_VERMELHO "Cliente nao encontrado.\n" COR_RESET);
    }
    printf(COR_VERDE "Cliente: %s\n\n" COR_RESET, clientes[idxCliente].nome);
    obterDataAtual(venda.data, sizeof(venda.data));

    /* 2) Itens: cada produto precisa existir e ter estoque suficiente. */
    printf("Informe os produtos (codigo 0 finaliza a lista; na quantidade, 0 cancela o item).\n\n");
    while (venda.numItens < MAX_ITENS_VENDA) {
        int cod = lerCodigo("Codigo do produto (0 finaliza): ");
        if (cod == 0) break;

        int idx = buscaBinariaProduto(produtos, 0, nProdutos - 1, cod);
        if (idx < 0) {
            printf(COR_VERMELHO "Produto nao encontrado.\n" COR_RESET);
            continue;
        }
        Produto *p = &produtos[idx];
        printf("  %s | R$ %.2f | disponivel: %d\n", p->nome, p->preco, p->quantidadeEstoque);
        if (p->quantidadeEstoque <= 0) {
            printf(COR_VERMELHO "  Produto sem estoque disponivel.\n" COR_RESET);
            continue;
        }

        int qtd;
        if (lerQuantidade("  Quantidade (0 cancela o item): ", &qtd) == LEITURA_CANCELADA) {
            printf(COR_AMARELO "  Item cancelado.\n\n" COR_RESET);
            continue;
        }
        if (qtd > p->quantidadeEstoque) {
            printf(COR_VERMELHO "  Estoque insuficiente! Disponivel: %d\n" COR_RESET,
                   p->quantidadeEstoque);
            continue;
        }

        /* Se o produto ja esta na venda, soma a quantidade ao item existente. */
        int i;
        for (i = 0; i < venda.numItens; i++) {
            if (venda.itens[i].codigoProduto == p->codigo) break;
        }
        ItemVenda *it = &venda.itens[i];
        if (i == venda.numItens) {
            it->codigoProduto = p->codigo;
            snprintf(it->nomeProduto, TAM_NOME_PROD, "%s", p->nome);
            it->precoUnitario = p->preco;
            it->quantidade = 0;
            venda.numItens++;
        }
        it->quantidade += qtd;
        it->subtotal = it->quantidade * it->precoUnitario;
        p->quantidadeEstoque -= qtd;      /* baixa (ainda so em memoria) */

        printf(COR_VERDE "  Item adicionado.\n\n" COR_RESET);
    }
    if (venda.numItens == MAX_ITENS_VENDA) {
        printf(COR_AMARELO "Limite de %d itens por venda atingido.\n" COR_RESET, MAX_ITENS_VENDA);
    }

    if (venda.numItens == 0) {
        printf(COR_AMARELO "\nNenhum item informado. Venda cancelada.\n" COR_RESET);
        return;
    }

    /* 3) Total (calculado recursivamente) e confirmacao. */
    venda.valorTotal = totalItensRecursivo(venda.itens, venda.numItens);
    printf("\n");
    exibirVenda(&venda, clientes[idxCliente].nome);

    if (!confirmar("\nFinalizar a venda? (S/N): ")) {
        printf(COR_AMARELO "Venda cancelada. O estoque nao foi alterado.\n" COR_RESET);
        return;
    }

    /* 4) Grava estoque atualizado e depois a venda. */
    if (!salvarProdutos(produtos, nProdutos)) {
        printf(COR_VERMELHO "Erro ao atualizar o estoque. Venda nao registrada.\n" COR_RESET);
        return;
    }
    if (!anexarRegistro(ARQ_VENDAS, &venda, sizeof(Venda))) {
        printf(COR_VERMELHO "Erro ao gravar a venda no arquivo.\n" COR_RESET);
        return;
    }
    printf(COR_VERDE "\nVenda registrada e estoque atualizado com sucesso!\n" COR_RESET);
}

void registrarVenda(void) {
    imprimirCabecalho("REGISTRAR VENDA", COR_AZUL);

    int nClientes, nProdutos, nVendas;
    Cliente *clientes = carregarClientes(&nClientes);
    Produto *produtos = carregarProdutos(&nProdutos);
    Venda   *vendas   = carregarVendas(&nVendas);

    if (clientes == NULL || produtos == NULL || vendas == NULL) {
        printf(COR_VERMELHO "Erro ao ler os arquivos de dados.\n" COR_RESET);
    } else if (nClientes == 0) {
        printf(COR_AMARELO "\nCadastre ao menos um cliente antes de vender.\n" COR_RESET);
    } else if (nProdutos == 0) {
        printf(COR_AMARELO "\nCadastre ao menos um produto antes de vender.\n" COR_RESET);
    } else {
        /* Codigo da venda gerado automaticamente: maior codigo + 1. */
        int proximo = 1;
        for (int i = 0; i < nVendas; i++) {
            if (vendas[i].codigo >= proximo) proximo = vendas[i].codigo + 1;
        }
        processarVenda(clientes, nClientes, produtos, nProdutos, proximo);
    }

    free(clientes);
    free(produtos);
    free(vendas);
}

/* ==========================================================
   Consultas
   ========================================================== */

void listarVendas(void) {
    imprimirCabecalho("LISTAR VENDAS", COR_AZUL);

    int nVendas, nClientes;
    Venda *vendas = carregarVendas(&nVendas);
    Cliente *clientes = carregarClientes(&nClientes);

    if (vendas == NULL || clientes == NULL) {
        printf(COR_VERMELHO "Erro ao ler os arquivos de dados.\n" COR_RESET);
    } else if (nVendas == 0) {
        printf(COR_AMARELO "\nNenhuma venda registrada.\n" COR_RESET);
    } else {
        printf("\n" COR_NEGRITO "%-7s %-11s %-24s %6s %12s\n" COR_RESET,
               "VENDA", "DATA", "CLIENTE", "ITENS", "TOTAL");
        double soma = 0.0;
        for (int i = 0; i < nVendas; i++) {
            printf("%-7d %-11s %-24.24s %6d R$%10.2f\n", vendas[i].codigo, vendas[i].data,
                   nomeDoCliente(clientes, nClientes, vendas[i].codigoCliente),
                   vendas[i].numItens, vendas[i].valorTotal);
            soma += vendas[i].valorTotal;
        }
        printf(COR_VERDE "\n%d venda(s) | Total vendido: R$ %.2f\n" COR_RESET, nVendas, soma);
        printf("Use a opcao 3 do menu para ver os itens de uma venda.\n");
    }
    free(vendas);
    free(clientes);
}

void buscarVenda(void) {
    imprimirCabecalho("CONSULTAR VENDA", COR_AZUL);

    int nVendas, nClientes;
    Venda *vendas = carregarVendas(&nVendas);
    Cliente *clientes = carregarClientes(&nClientes);

    if (vendas == NULL || clientes == NULL) {
        printf(COR_VERMELHO "Erro ao ler os arquivos de dados.\n" COR_RESET);
    } else {
        int cod = lerCodigo("Codigo da venda (0 cancela): ");
        if (cod != 0) {
            int achou = 0;
            for (int i = 0; i < nVendas; i++) {
                if (vendas[i].codigo == cod) {
                    printf("\n");
                    exibirVenda(&vendas[i],
                                nomeDoCliente(clientes, nClientes, vendas[i].codigoCliente));
                    achou = 1;
                    break;
                }
            }
            if (!achou) printf(COR_VERMELHO "\nVenda nao encontrada.\n" COR_RESET);
        }
    }
    free(vendas);
    free(clientes);
}
