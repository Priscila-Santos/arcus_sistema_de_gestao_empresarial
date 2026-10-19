#ifndef ARQUIVO_H
#define ARQUIVO_H

#include <stddef.h>

/* ==========================================================
   arquivo.h
   Camada de persistencia: leitura e gravacao de vetores de
   struct em arquivos BINARIOS (fread / fwrite). As funcoes
   sao genericas (void *) e servem para Cliente, Produto e Venda.
   Os caminhos sao relativos a pasta de onde o programa e executado
   (execute sempre a partir da raiz do projeto).
   ========================================================== */

#define DIR_DADOS    "data"
#define ARQ_CLIENTES "data/clientes.dat"
#define ARQ_PRODUTOS "data/produtos.dat"
#define ARQ_VENDAS   "data/vendas.dat"

/* Carrega TODOS os registros do arquivo para um vetor alocado com
   malloc. O numero de registros lidos e devolvido em *total (passagem
   por referencia). Arquivo inexistente = vetor vazio (*total = 0).
   Retorna NULL em caso de erro. O chamador deve usar free(). */
void *carregarRegistros(const char *caminho, size_t tamanhoRegistro, int *total);

/* Regrava o arquivo inteiro com 'total' registros. Escreve primeiro em um
   arquivo temporario e so depois substitui o original (evita perder dados
   se a gravacao falhar no meio). Retorna 1 em sucesso, 0 em erro. */
int salvarRegistros(const char *caminho, const void *dados,
                    size_t tamanhoRegistro, int total);

/* Acrescenta um unico registro ao final do arquivo. 1 = ok, 0 = erro. */
int anexarRegistro(const char *caminho, const void *registro, size_t tamanhoRegistro);

#endif
