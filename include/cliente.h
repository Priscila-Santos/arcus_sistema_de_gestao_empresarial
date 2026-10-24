#ifndef CLIENTE_H
#define CLIENTE_H

/* ==========================================================
   cliente.h
   Modulo responsavel pelo cadastro de clientes (CRUD completo,
   com persistencia em data/clientes.dat).
   ========================================================== */

#define TAM_NOME     60
#define TAM_CPF      15
#define TAM_TELEFONE 20
#define TAM_EMAIL    60
#define TAM_CIDADE   40

/* 1 = valida os digitos verificadores do CPF; 0 = so exige 11 digitos
   (util para testes com CPFs ficticios). */
#define VALIDAR_DIGITOS_CPF 1

typedef struct {
    int  codigo;
    char nome[TAM_NOME];
    char cpf[TAM_CPF];          /* guardado somente com os 11 digitos */
    char telefone[TAM_TELEFONE];
    char email[TAM_EMAIL];
    char cidade[TAM_CIDADE];
} Cliente;

/* Submenu de Clientes (chamado pelo main). */
void menuClientes(void);

/* Operacoes do CRUD. */
void cadastrarCliente(void);
void listarClientes(void);
void buscarCliente(void);
void alterarCliente(void);
void excluirCliente(void);

/* ---------- Funcoes de apoio usadas tambem por outros modulos ---------- */

/* Carrega todos os clientes (vetor malloc, ORDENADO por codigo).
   O total volta por referencia. NULL = erro. Use free(). */
Cliente *carregarClientes(int *total);

/* Ordena por codigo e grava o vetor inteiro no arquivo. 1 = ok. */
int salvarClientes(Cliente *v, int total);

/* RECURSAO: busca binaria recursiva por codigo em vetor ordenado.
   Retorna o indice encontrado ou -1. Chama com ini = 0, fim = total - 1. */
int buscaBinariaCliente(const Cliente *v, int ini, int fim, int codigo);

/* Valida CPF (11 digitos; confere digitos verificadores se habilitado). */
int validarCPF(const char *cpfSomenteDigitos);

#endif
