#include <stdio.h>
#include "../include/utils.h"
#include "../include/cliente.h"
#include "../include/produto.h"
#include "../include/venda.h"
#include "../include/relatorio.h"

/* ==========================================================
   main.c
   ARCUS - Sistema de Gerenciamento Empresarial
   Algoritmos e Programacao II - UFRB/CETENS - EaD

   Aluna: Priscila Silva dos Santos

   main() apenas coordena o fluxo do menu principal. Toda a
   logica fica nos modulos:
     cliente.c   - cadastro de clientes
     produto.c   - cadastro de produtos e estoque
     venda.c     - registro de vendas (baixa de estoque)
     relatorio.c - consultas, relatorios e estatisticas
     arquivo.c   - leitura/gravacao em arquivos binarios
     utils.c     - tela, cores, leitura e validacao de dados
   ========================================================== */

/* Desenho das letras do logotipo "ARCUS": 5 letras x 7 linhas x 7 colunas. */
static const char *LETRAS[5][7] = {
    { "  ###  ", " #   # ", "#     #", "#######", "#     #", "#     #", "#     #" },  /* A */
    { "###### ", "#     #", "#     #", "###### ", "#   #  ", "#    # ", "#     #" },  /* R */
    { " ##### ", "#     #", "#      ", "#      ", "#      ", "#     #", " ##### " },  /* C */
    { "#     #", "#     #", "#     #", "#     #", "#     #", "#     #", " ##### " },  /* U */
    { " ##### ", "#     #", "#      ", " ##### ", "      #", "#     #", " ##### " }   /* S */
};

static void desenharLogo(const char *cores[5], int recuo);
static void exibirBanner(void);

int main(void) {
    int opcao;

    inicializarTerminal();
    exibirBanner();          /* 1a tela: abertura */
    pausar();

    do {
        /* 2a tela: o nome ARCUS aparece de novo, agora no menu principal. */
        const char *verde[5] = { COR_VERDE, COR_VERDE, COR_VERDE, COR_VERDE, COR_VERDE };

        imprimirCabecalho("ARCUS  -  MENU PRINCIPAL", COR_VERDE);
        printf("\n");
        desenharLogo(verde, 5);
        printf(COR_VERDE "\n            Sistema de Gestao Empresarial\n" COR_RESET "\n");
        printf(COR_NEGRITO COR_CIANO "  1" COR_RESET " - Gerenciar Clientes\n");
        printf(COR_NEGRITO COR_CIANO "  2" COR_RESET " - Gerenciar Produtos\n");
        printf(COR_NEGRITO COR_CIANO "  3" COR_RESET " - Gerenciar Vendas\n");
        printf(COR_NEGRITO COR_CIANO "  4" COR_RESET " - Consultas e Relatorios\n");
        printf(COR_NEGRITO COR_CIANO "  5" COR_RESET " - Estatisticas\n");
        printf(COR_NEGRITO COR_VERMELHO "  0" COR_RESET " - Sair\n");
        imprimirLinha(COR_VERDE);

        opcao = lerInteiro(COR_NEGRITO ">> Escolha uma opcao: " COR_RESET);

        switch (opcao) {
            case 1: menuClientes();     break;
            case 2: menuProdutos();     break;
            case 3: menuVendas();       break;
            case 4: menuRelatorios();   break;
            case 5: menuEstatisticas(); break;
            case 0:
                imprimirCabecalho("ATE LOGO!", COR_VERDE);
                printf(COR_VERDE "\nObrigado por usar o Arcus!\n\n" COR_RESET);
                break;
            default:
                printf(COR_VERMELHO "\nOpcao invalida! Tente novamente.\n" COR_RESET);
                pausar();
        }
    } while (opcao != 0);

    return 0;
}

/* Imprime o logotipo linha por linha: para cada linha, percorre as 5 letras
   e pinta cada uma com a sua cor. 'recuo' = espacos a esquerda (centralizar). */
static void desenharLogo(const char *cores[5], int recuo) {
    for (int linha = 0; linha < 7; linha++) {
        printf("%*s", recuo, "");
        for (int letra = 0; letra < 5; letra++) {
            printf("%s" COR_NEGRITO "%s" COR_RESET "  ", cores[letra], LETRAS[letra][linha]);
        }
        printf("\n");
    }
}

/* Tela de abertura: logotipo colorido + mensagem de boas-vindas. */
static void exibirBanner(void) {
    const char *cores[5] = { COR_CIANO, COR_AZUL, COR_MAGENTA, COR_AMARELO, COR_VERDE };

    limparTela();
    imprimirLinha(COR_CIANO);
    printf("\n");
    desenharLogo(cores, 5);
    printf("\n");
    printf("    " COR_NEGRITO "Bem Vindo(a) ao " COR_AMARELO "SISTEMA DE GESTAO EMPRESARIAL" COR_RESET "\n");
    printf("\n");
    printf(COR_CIANO "      Algoritmos e Programacao II - UFRB/CETENS\n" COR_RESET);
    imprimirLinha(COR_CIANO);
}
