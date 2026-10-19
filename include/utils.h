#ifndef UTILS_H
#define UTILS_H

/* ==========================================================
   utils.h
   Funcoes utilitarias de interface: cores no terminal,
   limpeza de tela, pausa, leitura segura de dados e
   validacao basica de entrada (strings, caracteres, numeros).
   ========================================================== */

/* Codigos de cor ANSI */
#define COR_RESET    "\x1b[0m"
#define COR_VERDE    "\x1b[32m"
#define COR_CIANO    "\x1b[36m"
#define COR_AMARELO  "\x1b[33m"
#define COR_VERMELHO "\x1b[31m"
#define COR_AZUL     "\x1b[34m"
#define COR_MAGENTA  "\x1b[35m"
#define COR_NEGRITO  "\x1b[1m"

/* ---------- Terminal / interface ---------- */
void inicializarTerminal(void);          /* habilita cores ANSI no Windows */
void limparTela(void);
void pausar(void);
void imprimirLinha(const char *cor);
void imprimirCabecalho(const char *titulo, const char *cor);
void avisoEmDesenvolvimento(const char *nomeFuncionalidade);

/* ---------- Padrao de CANCELAMENTO ----------
   Em qualquer campo do sistema, digitar 0 cancela a operacao em andamento
   (nada e salvo). As funcoes de leitura abaixo devolvem um destes codigos: */
#define LEITURA_CANCELADA (-1)  /* o usuario digitou 0                      */
#define LEITURA_VAZIA     0     /* ENTER em campo opcional (manter o atual)  */
#define LEITURA_OK        1     /* valor lido e valido                       */

/* Imprime a dica "digite 0 em qualquer campo para cancelar". */
void avisoCancelar(void);

/* ---------- Leitura segura ---------- */
void  lerLinha(char *destino, int tamanho);
int   lerInteiro(const char *mensagem);
float lerFloat(const char *mensagem);          /* aceita 19.90 ou 19,90 */
int   lerCodigo(const char *mensagem);         /* repete ate ser >= 0 (0 = cancelar) */
int   confirmar(const char *pergunta);         /* S/N -> 1 ou 0 */

/* Quantidade > 0, devolvida por REFERENCIA em *valor.
   Retorna LEITURA_OK ou LEITURA_CANCELADA (usuario digitou 0). */
int lerQuantidade(const char *mensagem, int *valor);

/* Versoes "opcionais": ENTER vazio significa "manter valor atual".
   Retornam LEITURA_OK (valor lido), LEITURA_VAZIA (ENTER) ou
   LEITURA_CANCELADA (0). O destino so e alterado em LEITURA_OK. */
int lerInteiroOpcional(const char *mensagem, int *valor);
int lerFloatOpcional(const char *mensagem, float *valor);
int lerTextoOpcional(const char *mensagem, char *destino, int tamanho);

/* Texto obrigatorio (nao aceita vazio). Retorna LEITURA_OK ou LEITURA_CANCELADA. */
int lerTextoObrigatorio(const char *mensagem, char *destino, int tamanho);

/* Funcao-base de leitura de texto. Valida usando uma FUNCAO PASSADA COMO
   ARGUMENTO (ponteiro para funcao); se 'validar' for NULL, aceita qualquer
   texto nao vazio. Se 'opcional' = 1, ENTER vazio retorna LEITURA_VAZIA. */
int lerCampoValidado(const char *mensagem, char *destino, int tamanho,
                     int (*validar)(const char *), const char *mensagemErro,
                     int opcional);

/* ---------- Strings e caracteres ---------- */
void removerEspacos(char *texto);                         /* trim */
void apenasDigitos(const char *origem, char *destino, int tamanho);
int  contemIgnorandoCaso(const char *texto, const char *trecho);

/* ---------- Data ---------- */
void obterDataAtual(char *destino, int tamanho);          /* dd/mm/aaaa */

#endif
