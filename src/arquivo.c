#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include "../include/arquivo.h"

#ifdef _WIN32
#include <direct.h>
#define CRIAR_PASTA(d) _mkdir(d)
#else
#include <sys/stat.h>
#define CRIAR_PASTA(d) mkdir(d, 0755)
#endif

/* Garante que a pasta data/ exista (se ja existir, o erro e ignorado). */
static void garantirPastaDados(void) {
    CRIAR_PASTA(DIR_DADOS);
}

void *carregarRegistros(const char *caminho, size_t tamanhoRegistro, int *total) {
    *total = 0;
    FILE *f = fopen(caminho, "rb");
    if (f == NULL) {
        if (errno == ENOENT) {
            return malloc(tamanhoRegistro);   /* ainda nao existe: lista vazia */
        }
        return NULL;
    }

    fseek(f, 0, SEEK_END);
    long bytes = ftell(f);
    rewind(f);

    int n = (bytes > 0) ? (int)(bytes / (long)tamanhoRegistro) : 0;
    void *dados = malloc((n > 0 ? (size_t)n : 1) * tamanhoRegistro);
    if (dados == NULL) {
        fclose(f);
        return NULL;
    }
    if (n > 0 && fread(dados, tamanhoRegistro, (size_t)n, f) != (size_t)n) {
        free(dados);
        fclose(f);
        return NULL;
    }
    fclose(f);
    *total = n;
    return dados;
}

int salvarRegistros(const char *caminho, const void *dados,
                    size_t tamanhoRegistro, int total) {
    char temporario[256];
    snprintf(temporario, sizeof temporario, "%s.tmp", caminho);

    garantirPastaDados();
    FILE *f = fopen(temporario, "wb");
    if (f == NULL) return 0;

    if (total > 0 && fwrite(dados, tamanhoRegistro, (size_t)total, f) != (size_t)total) {
        fclose(f);
        remove(temporario);
        return 0;
    }
    if (fclose(f) != 0) {
        remove(temporario);
        return 0;
    }
    remove(caminho);   /* no Windows, rename falha se o destino existir */
    return rename(temporario, caminho) == 0;
}

int anexarRegistro(const char *caminho, const void *registro, size_t tamanhoRegistro) {
    garantirPastaDados();
    FILE *f = fopen(caminho, "ab");
    if (f == NULL) return 0;
    size_t escritos = fwrite(registro, tamanhoRegistro, 1, f);
    fclose(f);
    return escritos == 1;
}
