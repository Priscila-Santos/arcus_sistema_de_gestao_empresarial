#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <time.h>
#include "../include/utils.h"

#ifdef _WIN32
#include <windows.h>
#ifndef ENABLE_VIRTUAL_TERMINAL_PROCESSING
#define ENABLE_VIRTUAL_TERMINAL_PROCESSING 0x0004
#endif
#endif

/* ================= Terminal / interface ================= */

/* No Windows muito antigo (sem suporte a ANSI) volta a usar "cls". */
static int usarCls = 0;

void inicializarTerminal(void) {
#ifdef _WIN32
    /* Ativa o processamento de sequencias ANSI no console do Windows,
       para que cores e limpeza de tela funcionem no PowerShell/CMD. */
    HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD modo = 0;
    if (h != INVALID_HANDLE_VALUE && GetConsoleMode(h, &modo)) {
        if (!SetConsoleMode(h, modo | ENABLE_VIRTUAL_TERMINAL_PROCESSING)) {
            usarCls = 1;
        }
    }
#endif
}

/* Limpa a tela com sequencias ANSI (home + apagar tela + apagar historico).
   Vantagem sobre system("cls"/"clear"): funciona igual no PowerShell, no
   terminal MSYS2 (mintty), no Linux e no macOS, e nao abre outro processo. */
void limparTela(void) {
    if (usarCls) {
        system("cls");
        return;
    }
    printf("\x1b[H\x1b[2J\x1b[3J");
    fflush(stdout);
}

void pausar(void) {
    printf(COR_AMARELO "\n>> Pressione ENTER para continuar..." COR_RESET);
    /* Toda leitura do programa usa fgets (que ja consome o '\n'),
       portanto basta esperar uma unica linha aqui. */
    int c;
    while ((c = getchar()) != '\n' && c != EOF) { /* descarta */ }
}

void imprimirLinha(const char *cor) {
    printf("%s", cor);
    for (int i = 0; i < 54; i++) {
        printf("=");
    }
    printf("%s\n", COR_RESET);
}

void imprimirCabecalho(const char *titulo, const char *cor) {
    limparTela();
    imprimirLinha(cor);
    printf("%s%s", cor, COR_NEGRITO);
    printf("   %s\n", titulo);
    printf("%s", COR_RESET);
    imprimirLinha(cor);
}

void avisoEmDesenvolvimento(const char *nomeFuncionalidade) {
    printf("\n");
    imprimirLinha(COR_AMARELO);
    printf(COR_AMARELO COR_NEGRITO " [!] " COR_RESET COR_AMARELO
           "Funcionalidade \"%s\" ainda sera implementada.\n"
           COR_RESET, nomeFuncionalidade);
    imprimirLinha(COR_AMARELO);
}

/* ================= Strings e caracteres ================= */

void removerEspacos(char *texto) {
    size_t ini = 0, fim = strlen(texto);
    while (texto[ini] != '\0' && isspace((unsigned char)texto[ini])) ini++;
    while (fim > ini && isspace((unsigned char)texto[fim - 1])) fim--;
    memmove(texto, texto + ini, fim - ini);
    texto[fim - ini] = '\0';
}

void apenasDigitos(const char *origem, char *destino, int tamanho) {
    int j = 0;
    for (; *origem != '\0' && j < tamanho - 1; origem++) {
        if (isdigit((unsigned char)*origem)) {
            destino[j++] = *origem;
        }
    }
    destino[j] = '\0';
}

int contemIgnorandoCaso(const char *texto, const char *trecho) {
    size_t n = strlen(trecho);
    if (n == 0) return 1;
    for (; *texto != '\0'; texto++) {
        size_t i = 0;
        while (i < n && texto[i] != '\0' &&
               tolower((unsigned char)texto[i]) == tolower((unsigned char)trecho[i])) {
            i++;
        }
        if (i == n) return 1;
    }
    return 0;
}

/* ================= Leitura segura ================= */

void lerLinha(char *destino, int tamanho) {
    if (fgets(destino, tamanho, stdin) != NULL) {
        size_t len = strlen(destino);
        if (len > 0 && destino[len - 1] == '\n') {
            destino[len - 1] = '\0';
        } else {
            /* Digitou mais do que cabe: descarta o restante da linha. */
            int c;
            while ((c = getchar()) != '\n' && c != EOF) { /* descarta */ }
        }
    } else {
        /* EOF (Ctrl+D / Ctrl+Z ou entrada redirecionada acabou):
           encerra de forma limpa em vez de ficar em laco infinito. */
        destino[0] = '\0';
        printf(COR_AMARELO "\n[Entrada encerrada. Finalizando o programa.]\n" COR_RESET);
        exit(0);
    }
}

/* Converte texto em int validando TODO o conteudo ("12abc" e invalido). */
static int converterInteiro(const char *texto, int *valor) {
    char *fim;
    errno = 0;
    long v = strtol(texto, &fim, 10);
    if (fim == texto || errno == ERANGE || v > INT_MAX || v < INT_MIN) return 0;
    while (*fim != '\0') {
        if (!isspace((unsigned char)*fim)) return 0;
        fim++;
    }
    *valor = (int)v;
    return 1;
}

/* Converte texto em float; aceita virgula como separador decimal. */
static int converterFloat(const char *texto, float *valor) {
    char copia[64];
    snprintf(copia, sizeof copia, "%s", texto);
    for (char *p = copia; *p != '\0'; p++) {
        if (*p == ',') *p = '.';
        else if (!isdigit((unsigned char)*p) && *p != '.' && *p != '-' &&
                 *p != '+' && !isspace((unsigned char)*p)) {
            return 0;   /* rejeita letras, "nan", "inf", etc. */
        }
    }
    char *fim;
    double v = strtod(copia, &fim);
    if (fim == copia) return 0;
    while (*fim != '\0') {
        if (!isspace((unsigned char)*fim)) return 0;
        fim++;
    }
    *valor = (float)v;
    return 1;
}

int lerInteiro(const char *mensagem) {
    char buffer[64];
    int valor;
    while (1) {
        printf("%s", mensagem);
        lerLinha(buffer, sizeof(buffer));
        if (converterInteiro(buffer, &valor)) return valor;
        printf(COR_VERMELHO "Entrada invalida! Digite um numero inteiro.\n" COR_RESET);
    }
}

float lerFloat(const char *mensagem) {
    char buffer[64];
    float valor;
    while (1) {
        printf("%s", mensagem);
        lerLinha(buffer, sizeof(buffer));
        if (converterFloat(buffer, &valor)) return valor;
        printf(COR_VERMELHO "Entrada invalida! Digite um valor numerico (ex: 19.90).\n" COR_RESET);
    }
}

int lerCodigo(const char *mensagem) {
    int v;
    while (1) {
        v = lerInteiro(mensagem);
        if (v >= 0) return v;
        printf(COR_VERMELHO "O codigo nao pode ser negativo.\n" COR_RESET);
    }
}

int lerQuantidade(const char *mensagem, int *valor) {
    int v;
    while (1) {
        v = lerInteiro(mensagem);
        if (v == 0) return LEITURA_CANCELADA;
        if (v > 0) {
            *valor = v;
            return LEITURA_OK;
        }
        printf(COR_VERMELHO "A quantidade deve ser maior que zero.\n" COR_RESET);
    }
}

int confirmar(const char *pergunta) {
    char buffer[16];
    while (1) {
        printf("%s", pergunta);
        lerLinha(buffer, sizeof(buffer));
        removerEspacos(buffer);
        char c = (char)toupper((unsigned char)buffer[0]);
        if (c == 'S') return 1;
        if (c == 'N') return 0;
        printf(COR_VERMELHO "Responda com S (sim) ou N (nao).\n" COR_RESET);
    }
}

void avisoCancelar(void) {
    printf(COR_AMARELO "Dica: digite 0 em qualquer campo para CANCELAR a operacao.\n\n" COR_RESET);
}

int lerInteiroOpcional(const char *mensagem, int *valor) {
    char buffer[64];
    int v;
    while (1) {
        printf("%s", mensagem);
        lerLinha(buffer, sizeof(buffer));
        removerEspacos(buffer);
        if (buffer[0] == '\0') return LEITURA_VAZIA;
        if (converterInteiro(buffer, &v)) {
            if (v == 0) return LEITURA_CANCELADA;
            *valor = v;
            return LEITURA_OK;
        }
        printf(COR_VERMELHO "Entrada invalida! Digite um numero inteiro, ENTER ou 0.\n" COR_RESET);
    }
}

int lerFloatOpcional(const char *mensagem, float *valor) {
    char buffer[64];
    float v;
    while (1) {
        printf("%s", mensagem);
        lerLinha(buffer, sizeof(buffer));
        removerEspacos(buffer);
        if (buffer[0] == '\0') return LEITURA_VAZIA;
        if (converterFloat(buffer, &v)) {
            if (v == 0.0f) return LEITURA_CANCELADA;
            *valor = v;
            return LEITURA_OK;
        }
        printf(COR_VERMELHO "Entrada invalida! Digite um valor numerico, ENTER ou 0.\n" COR_RESET);
    }
}

int lerCampoValidado(const char *mensagem, char *destino, int tamanho,
                     int (*validar)(const char *), const char *mensagemErro,
                     int opcional) {
    char buffer[128];
    while (1) {
        printf("%s", mensagem);
        lerLinha(buffer, sizeof(buffer));
        removerEspacos(buffer);

        if (strcmp(buffer, "0") == 0) return LEITURA_CANCELADA;   /* saida a qualquer momento */
        if (buffer[0] == '\0') {
            if (opcional) return LEITURA_VAZIA;                    /* manter valor atual */
            printf(COR_VERMELHO "Este campo e obrigatorio.\n" COR_RESET);
            continue;
        }
        if (validar == NULL || validar(buffer)) {
            snprintf(destino, (size_t)tamanho, "%s", buffer);      /* so grava se valido */
            return LEITURA_OK;
        }
        printf(COR_VERMELHO "%s\n" COR_RESET, mensagemErro);
    }
}

int lerTextoObrigatorio(const char *mensagem, char *destino, int tamanho) {
    return lerCampoValidado(mensagem, destino, tamanho, NULL, "", 0);
}

int lerTextoOpcional(const char *mensagem, char *destino, int tamanho) {
    return lerCampoValidado(mensagem, destino, tamanho, NULL, "", 1);
}

/* ================= Data ================= */

void obterDataAtual(char *destino, int tamanho) {
    time_t agora = time(NULL);
    struct tm *info = localtime(&agora);
    strftime(destino, (size_t)tamanho, "%d/%m/%Y", info);
}
