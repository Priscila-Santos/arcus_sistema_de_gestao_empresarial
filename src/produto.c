#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/produto.h"
#include "../include/utils.h"
#include "../include/arquivo.h"
#include "../include/venda.h"

/* ==========================================================
   Persistencia, busca e recursao
   ========================================================== */

static int compararProdutos(const void *a, const void *b) {
    const Produto *x = (const Produto *)a;
    const Produto *y = (const Produto *)b;
    return (x->codigo > y->codigo) - (x->codigo < y->codigo);
}

Produto *carregarProdutos(int *total) {
    Produto *v = (Produto *)carregarRegistros(ARQ_PRODUTOS, sizeof(Produto), total);
    if (v != NULL && *total > 1) {
        qsort(v, (size_t)*total, sizeof(Produto), compararProdutos);
    }
    return v;
}

int salvarProdutos(Produto *v, int total) {
    if (total > 1) {
        qsort(v, (size_t)total, sizeof(Produto), compararProdutos);
    }
    return salvarRegistros(ARQ_PRODUTOS, v, sizeof(Produto), total);
}

/* Busca binaria RECURSIVA (vetor ordenado por codigo). */
int buscaBinariaProduto(const Produto *v, int ini, int fim, int codigo) {
    if (ini > fim) return -1;
    int meio = ini + (fim - ini) / 2;
    if (v[meio].codigo == codigo) return meio;
    if (codigo < v[meio].codigo) {
        return buscaBinariaProduto(v, ini, meio - 1, codigo);
    }
    return buscaBinariaProduto(v, meio + 1, fim, codigo);
}

/* RECURSAO: valor do estoque dos n primeiros produtos =
   valor do n-esimo produto + valor dos n-1 anteriores.
   Caso base: n <= 0 -> 0. */
double valorEstoqueRecursivo(const Produto *v, int n) {
    if (n <= 0) return 0.0;
    return (double)v[n - 1].preco * v[n - 1].quantidadeEstoque
           + valorEstoqueRecursivo(v, n - 1);
}

/* ==========================================================
   Exibicao
   ========================================================== */

static void cabecalhoTabela(void) {
    printf(COR_MAGENTA COR_NEGRITO "%-6s %-26s %-14s %10s %8s\n" COR_RESET,
           "COD", "NOME", "CATEGORIA", "PRECO", "ESTOQUE");
}

static void linhaProduto(const Produto *p) {
    printf("%-6d %-26.26s %-14.14s R$%8.2f %8d\n",
           p->codigo, p->nome, p->categoria, p->preco, p->quantidadeEstoque);
}

/* Preco precisa ser maior que zero (nao aceita negativo). Digitar 0 cancela.
   Devolve o valor por REFERENCIA em *destino; retorna LEITURA_OK ou LEITURA_CANCELADA. */
static int pedirPreco(const char *mensagem, float *destino) {
    while (1) {
        float v = lerFloat(mensagem);
        if (v == 0.0f) return LEITURA_CANCELADA;
        if (v > 0.0f) {
            *destino = v;
            return LEITURA_OK;
        }
        printf(COR_VERMELHO "O preco nao pode ser negativo.\n" COR_RESET);
    }
}

/* Versoes para ALTERACAO: ENTER mantem o valor atual (LEITURA_VAZIA). */
static int pedirPrecoOpcional(float *destino) {
    while (1) {
        float v;
        int r = lerFloatOpcional("Novo preco (R$): ", &v);
        if (r != LEITURA_OK) return r;            /* ENTER ou cancelar */
        if (v > 0.0f) {
            *destino = v;
            return LEITURA_OK;
        }
        printf(COR_VERMELHO "O preco nao pode ser negativo.\n" COR_RESET);
    }
}

static int pedirQuantidadeOpcional(int *destino) {
    while (1) {
        int v;
        int r = lerInteiroOpcional("Nova quantidade em estoque: ", &v);
        if (r != LEITURA_OK) return r;
        if (v > 0) {
            *destino = v;
            return LEITURA_OK;
        }
        printf(COR_VERMELHO "A quantidade deve ser maior que zero.\n" COR_RESET);
    }
}

/* ==========================================================
   Menu
   ========================================================== */

void menuProdutos(void) {
    int opcao;

    do {
        imprimirCabecalho("MODULO DE PRODUTOS", COR_MAGENTA);
        printf(COR_MAGENTA "\n  1" COR_RESET " - Cadastrar produto\n");
        printf(COR_MAGENTA "  2" COR_RESET " - Listar produtos\n");
        printf(COR_MAGENTA "  3" COR_RESET " - Buscar produto\n");
        printf(COR_MAGENTA "  4" COR_RESET " - Alterar produto\n");
        printf(COR_MAGENTA "  5" COR_RESET " - Excluir produto\n");
        printf(COR_MAGENTA "  6" COR_RESET " - Consultar estoque\n");
        printf(COR_MAGENTA "  7" COR_RESET " - Produtos com estoque baixo\n");
        printf(COR_VERMELHO "  0" COR_RESET " - Voltar ao menu principal\n");
        imprimirLinha(COR_MAGENTA);

        opcao = lerInteiro(COR_NEGRITO ">> Escolha uma opcao: " COR_RESET);

        switch (opcao) {
            case 1: cadastrarProduto();     pausar(); break;
            case 2: listarProdutos();       pausar(); break;
            case 3: buscarProduto();        pausar(); break;
            case 4: alterarProduto();       pausar(); break;
            case 5: excluirProduto();       pausar(); break;
            case 6: consultarEstoque();     pausar(); break;
            case 7: produtosEstoqueBaixo(); pausar(); break;
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
   CRUD
   ========================================================== */

void cadastrarProduto(void) {
    imprimirCabecalho("CADASTRAR PRODUTO", COR_MAGENTA);
    printf("\n");
    avisoCancelar();

    int total;
    Produto *v = carregarProdutos(&total);
    if (v == NULL) {
        printf(COR_VERMELHO "Erro ao ler o arquivo de produtos.\n" COR_RESET);
        return;
    }

    Produto novo;
    memset(&novo, 0, sizeof(novo));

    while (1) {
        novo.codigo = lerCodigo("Codigo (0 cancela): ");
        if (novo.codigo == 0) {
            printf(COR_AMARELO "Cadastro cancelado.\n" COR_RESET);
            free(v);
            return;
        }
        if (buscaBinariaProduto(v, 0, total - 1, novo.codigo) == -1) break;
        printf(COR_VERMELHO "Ja existe um produto com o codigo %d.\n" COR_RESET, novo.codigo);
    }

    /* && interrompe a cadeia se algum campo for cancelado (0). */
    int ok = lerTextoObrigatorio("Nome: ", novo.nome, TAM_NOME_PROD) != LEITURA_CANCELADA
          && lerTextoObrigatorio("Categoria: ", novo.categoria, TAM_CATEGORIA) != LEITURA_CANCELADA
          && pedirPreco("Preco (R$, maior que zero): ", &novo.preco) != LEITURA_CANCELADA
          && lerQuantidade("Quantidade em estoque: ", &novo.quantidadeEstoque) != LEITURA_CANCELADA;
    if (!ok) {
        printf(COR_AMARELO "\nCadastro cancelado. Nada foi salvo.\n" COR_RESET);
        free(v);
        return;
    }

    Produto *aux = (Produto *)realloc(v, (size_t)(total + 1) * sizeof(Produto));
    if (aux == NULL) {
        printf(COR_VERMELHO "Memoria insuficiente.\n" COR_RESET);
        free(v);
        return;
    }
    v = aux;
    v[total] = novo;
    total++;

    if (salvarProdutos(v, total)) {
        printf(COR_VERDE "\nProduto cadastrado com sucesso!\n" COR_RESET);
    } else {
        printf(COR_VERMELHO "\nErro ao gravar o arquivo de produtos.\n" COR_RESET);
    }
    free(v);
}

void listarProdutos(void) {
    imprimirCabecalho("LISTAR PRODUTOS", COR_MAGENTA);

    int total;
    Produto *v = carregarProdutos(&total);
    if (v == NULL) {
        printf(COR_VERMELHO "Erro ao ler o arquivo de produtos.\n" COR_RESET);
        return;
    }
    if (total == 0) {
        printf(COR_AMARELO "\nNenhum produto cadastrado.\n" COR_RESET);
    } else {
        printf("\n");
        cabecalhoTabela();
        for (int i = 0; i < total; i++) {
            linhaProduto(&v[i]);
        }
        printf(COR_VERDE "\nTotal de produtos: %d\n" COR_RESET, total);
    }
    free(v);
}

void buscarProduto(void) {
    imprimirCabecalho("BUSCAR PRODUTO", COR_MAGENTA);

    int total;
    Produto *v = carregarProdutos(&total);
    if (v == NULL) {
        printf(COR_VERMELHO "Erro ao ler o arquivo de produtos.\n" COR_RESET);
        return;
    }

    printf("\n  1 - Por codigo\n  2 - Por nome (parte do nome)\n  3 - Por categoria\n  0 - Cancelar\n");
    int tipo = lerInteiro(">> Buscar por: ");
    if (tipo == 0) {
        free(v);
        return;
    }
    int achados = 0;

    if (tipo == 1) {
        int cod = lerCodigo("Codigo (0 cancela): ");
        if (cod == 0) { free(v); return; }
        int idx = buscaBinariaProduto(v, 0, total - 1, cod);   /* recursiva */
        if (idx >= 0) {
            printf("\n");
            cabecalhoTabela();
            linhaProduto(&v[idx]);
            achados = 1;
        }
    } else if (tipo == 2 || tipo == 3) {
        char trecho[TAM_NOME_PROD];
        if (lerTextoObrigatorio(tipo == 2 ? "Nome ou parte (0 cancela): " : "Categoria ou parte (0 cancela): ",
                                trecho, TAM_NOME_PROD) == LEITURA_CANCELADA) {
            free(v);
            return;
        }
        printf("\n");
        cabecalhoTabela();
        for (int i = 0; i < total; i++) {
            const char *campo = (tipo == 2) ? v[i].nome : v[i].categoria;
            if (contemIgnorandoCaso(campo, trecho)) {
                linhaProduto(&v[i]);
                achados++;
            }
        }
    } else {
        printf(COR_VERMELHO "\nOpcao invalida.\n" COR_RESET);
        free(v);
        return;
    }

    if (achados == 0) {
        printf(COR_AMARELO "\nNenhum produto encontrado.\n" COR_RESET);
    } else {
        printf(COR_VERDE "\nProdutos encontrados: %d\n" COR_RESET, achados);
    }
    free(v);
}

void alterarProduto(void) {
    imprimirCabecalho("ALTERAR PRODUTO", COR_MAGENTA);

    int total;
    Produto *v = carregarProdutos(&total);
    if (v == NULL) {
        printf(COR_VERMELHO "Erro ao ler o arquivo de produtos.\n" COR_RESET);
        return;
    }

    int cod = lerCodigo("Codigo do produto a alterar (0 cancela): ");
    if (cod == 0) { free(v); return; }

    int idx = buscaBinariaProduto(v, 0, total - 1, cod);
    if (idx < 0) {
        printf(COR_VERMELHO "\nProduto nao encontrado.\n" COR_RESET);
        free(v);
        return;
    }

    Produto *p = &v[idx];
    printf("\nDados atuais:\n");
    cabecalhoTabela();
    linhaProduto(p);
    printf(COR_AMARELO "\nENTER mantem o valor atual | 0 cancela a alteracao.\n\n" COR_RESET);

    /* Edita direto no registro (ponteiro p). Cancelou? descarta sem gravar. */
    int ok = lerTextoOpcional("Novo nome: ", p->nome, TAM_NOME_PROD) != LEITURA_CANCELADA
          && lerTextoOpcional("Nova categoria: ", p->categoria, TAM_CATEGORIA) != LEITURA_CANCELADA
          && pedirPrecoOpcional(&p->preco) != LEITURA_CANCELADA
          && pedirQuantidadeOpcional(&p->quantidadeEstoque) != LEITURA_CANCELADA;
    if (!ok) {
        printf(COR_AMARELO "\nAlteracao cancelada. Nada foi salvo.\n" COR_RESET);
        free(v);
        return;
    }

    if (salvarProdutos(v, total)) {
        printf(COR_VERDE "\nProduto alterado com sucesso!\n" COR_RESET);
    } else {
        printf(COR_VERMELHO "\nErro ao gravar o arquivo de produtos.\n" COR_RESET);
    }
    free(v);
}

void excluirProduto(void) {
    imprimirCabecalho("EXCLUIR PRODUTO", COR_MAGENTA);

    int total;
    Produto *v = carregarProdutos(&total);
    if (v == NULL) {
        printf(COR_VERMELHO "Erro ao ler o arquivo de produtos.\n" COR_RESET);
        return;
    }

    int cod = lerCodigo("Codigo do produto a excluir (0 cancela): ");
    if (cod == 0) { free(v); return; }

    int idx = buscaBinariaProduto(v, 0, total - 1, cod);
    if (idx < 0) {
        printf(COR_VERMELHO "\nProduto nao encontrado.\n" COR_RESET);
        free(v);
        return;
    }

    printf("\n");
    cabecalhoTabela();
    linhaProduto(&v[idx]);

    if (produtoTemVendas(cod)) {
        printf(COR_VERMELHO "\nExclusao negada: este produto consta em vendas registradas.\n" COR_RESET);
        free(v);
        return;
    }

    if (!confirmar("\nConfirma a exclusao? (S/N): ")) {
        printf(COR_AMARELO "Exclusao cancelada.\n" COR_RESET);
        free(v);
        return;
    }

    memmove(&v[idx], &v[idx + 1], (size_t)(total - idx - 1) * sizeof(Produto));
    total--;

    if (salvarProdutos(v, total)) {
        printf(COR_VERDE "\nProduto excluido com sucesso!\n" COR_RESET);
    } else {
        printf(COR_VERMELHO "\nErro ao gravar o arquivo de produtos.\n" COR_RESET);
    }
    free(v);
}

/* ==========================================================
   Estoque
   ========================================================== */

void consultarEstoque(void) {
    imprimirCabecalho("CONSULTAR ESTOQUE", COR_MAGENTA);

    int total;
    Produto *v = carregarProdutos(&total);
    if (v == NULL) {
        printf(COR_VERMELHO "Erro ao ler o arquivo de produtos.\n" COR_RESET);
        return;
    }
    if (total == 0) {
        printf(COR_AMARELO "\nNenhum produto cadastrado.\n" COR_RESET);
        free(v);
        return;
    }

    int cod = lerCodigo("Codigo do produto (0 = ver todo o estoque): ");
    printf("\n");

    if (cod != 0) {
        int idx = buscaBinariaProduto(v, 0, total - 1, cod);
        if (idx < 0) {
            printf(COR_VERMELHO "Produto nao encontrado.\n" COR_RESET);
        } else {
            const Produto *p = &v[idx];
            printf(COR_NEGRITO "%s" COR_RESET " (cod %d, %s)\n", p->nome, p->codigo, p->categoria);
            printf("Quantidade em estoque: %d unidade(s)\n", p->quantidadeEstoque);
            printf("Valor em estoque: R$ %.2f\n", (double)p->preco * p->quantidadeEstoque);
            if (p->quantidadeEstoque <= LIMITE_ESTOQUE_BAIXO) {
                printf(COR_VERMELHO "ATENCAO: estoque baixo!\n" COR_RESET);
            }
        }
    } else {
        printf(COR_MAGENTA COR_NEGRITO "%-6s %-26s %8s %10s %12s\n" COR_RESET,
               "COD", "NOME", "QTD", "PRECO", "VALOR");
        for (int i = 0; i < total; i++) {
            printf("%-6d %-26.26s %8d R$%8.2f R$%10.2f\n", v[i].codigo, v[i].nome,
                   v[i].quantidadeEstoque, v[i].preco,
                   (double)v[i].preco * v[i].quantidadeEstoque);
        }
        printf(COR_VERDE "\nValor total estimado do estoque: R$ %.2f\n" COR_RESET,
               valorEstoqueRecursivo(v, total));
    }
    free(v);
}

void imprimirProdutosEstoqueBaixo(void) {
    int total;
    Produto *v = carregarProdutos(&total);
    if (v == NULL) {
        printf(COR_VERMELHO "Erro ao ler o arquivo de produtos.\n" COR_RESET);
        return;
    }

    int achados = 0;
    for (int i = 0; i < total; i++) {
        if (v[i].quantidadeEstoque <= LIMITE_ESTOQUE_BAIXO) {
            if (achados == 0) cabecalhoTabela();
            linhaProduto(&v[i]);
            achados++;
        }
    }
    if (achados == 0) {
        printf(COR_VERDE "Nenhum produto com estoque baixo (limite: ate %d unidades).\n" COR_RESET,
               LIMITE_ESTOQUE_BAIXO);
    } else {
        printf(COR_VERMELHO "\n%d produto(s) com estoque baixo (limite: ate %d unidades).\n" COR_RESET,
               achados, LIMITE_ESTOQUE_BAIXO);
    }
    free(v);
}

void produtosEstoqueBaixo(void) {
    imprimirCabecalho("PRODUTOS COM ESTOQUE BAIXO", COR_MAGENTA);
    printf("\n");
    imprimirProdutosEstoqueBaixo();
}
