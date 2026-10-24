#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "../include/cliente.h"
#include "../include/utils.h"
#include "../include/arquivo.h"
#include "../include/venda.h"

/* ==========================================================
   Persistencia e busca
   ========================================================== */

static int compararClientes(const void *a, const void *b) {
    const Cliente *x = (const Cliente *)a;
    const Cliente *y = (const Cliente *)b;
    return (x->codigo > y->codigo) - (x->codigo < y->codigo);
}

Cliente *carregarClientes(int *total) {
    Cliente *v = (Cliente *)carregarRegistros(ARQ_CLIENTES, sizeof(Cliente), total);
    if (v != NULL && *total > 1) {
        qsort(v, (size_t)*total, sizeof(Cliente), compararClientes);
    }
    return v;
}

int salvarClientes(Cliente *v, int total) {
    if (total > 1) {
        qsort(v, (size_t)total, sizeof(Cliente), compararClientes);
    }
    return salvarRegistros(ARQ_CLIENTES, v, sizeof(Cliente), total);
}

/* Busca binaria RECURSIVA: a cada chamada o intervalo [ini, fim] cai
   pela metade. Caso base 1: intervalo vazio (nao achou).
   Caso base 2: o elemento do meio e o procurado. */
int buscaBinariaCliente(const Cliente *v, int ini, int fim, int codigo) {
    if (ini > fim) return -1;
    int meio = ini + (fim - ini) / 2;
    if (v[meio].codigo == codigo) return meio;
    if (codigo < v[meio].codigo) {
        return buscaBinariaCliente(v, ini, meio - 1, codigo);
    }
    return buscaBinariaCliente(v, meio + 1, fim, codigo);
}

/* ==========================================================
   Validacoes (strings e caracteres)
   ========================================================== */

/* Calcula UM digito verificador do CPF a partir dos 'n' primeiros digitos.
   Passo a passo (exemplo com os 9 primeiros digitos de 529.982.247-25):
     1) multiplica cada digito por um peso que comeca em n+1 e desce ate 2
          5*10 + 2*9 + 9*8 + 9*7 + 8*6 + 2*5 + 2*4 + 4*3 + 7*2 = 295
     2) resto = soma % 11                      ->  295 % 11 = 9
     3) se resto < 2 o digito e 0, senao e 11 - resto  ->  11 - 9 = 2
   Para o 2o digito repete-se com n = 10 (inclui o 1o digito, pesos 11 a 2).
    1) 5*11 + 2*10 + 9*9 + 9*8 + 8*7 + 2*6 + 2*5 + 4*4 + 7*3 + 2*2 = 347
    2) resto = 347 % 11 = 6
    3) resto >= 2, entao digito = 11 - 6 = 5
   Resultado final: 529.982.247-25
   */

static int calcularDigitoCPF(const char *cpf, int n) {
    int soma = 0;
    int peso = n + 1;
    for (int i = 0; i < n; i++) {
        soma += (cpf[i] - '0') * peso;
        peso--;
    }
    int resto = soma % 11;
    return (resto < 2) ? 0 : 11 - resto;
}

/* Um CPF e valido quando: (1) tem 11 digitos, (2) nao tem todos os digitos
   iguais e (3) os dois ultimos digitos conferem com o calculo acima. */
int validarCPF(const char *cpf) {
    if (strlen(cpf) != 11) return 0;
    for (int i = 0; i < 11; i++) {
        if (!isdigit((unsigned char)cpf[i])) return 0;
    }
#if VALIDAR_DIGITOS_CPF
    int todosIguais = 1;
    for (int i = 1; i < 11; i++) {
        if (cpf[i] != cpf[0]) { todosIguais = 0; break; }
    }
    if (todosIguais) return 0;                               /* 111.111.111-11 etc. */

    return calcularDigitoCPF(cpf, 9)  == cpf[9]  - '0'       /* 1o digito verificador */
        && calcularDigitoCPF(cpf, 10) == cpf[10] - '0';      /* 2o digito verificador */
#else
    return 1;
#endif
}

static int validarEmail(const char *e) {
    for (const char *p = e; *p != '\0'; p++) {
        if (isspace((unsigned char)*p)) return 0;
    }
    const char *arroba = strchr(e, '@');
    if (arroba == NULL || arroba == e) return 0;      /* precisa de '@' e texto antes */
    if (strchr(arroba + 1, '@') != NULL) return 0;    /* so um '@' */
    const char *ponto = strrchr(arroba + 1, '.');
    return ponto != NULL && ponto > arroba + 1 && *(ponto + 1) != '\0';
}

static int validarTelefone(const char *t) {
    int digitos = 0;
    for (; *t != '\0'; t++) {
        if (isdigit((unsigned char)*t)) digitos++;
        else if (strchr(" ()-+", *t) == NULL) return 0;
    }
    return digitos >= 8 && digitos <= 13;
}

static void formatarCPF(const char *d, char *saida) {
    snprintf(saida, TAM_CPF, "%.3s.%.3s.%.3s-%.2s", d, d + 3, d + 6, d + 9);
}

static int cpfJaCadastrado(const Cliente *v, int total, const char *cpf, int ignorar) {
    for (int i = 0; i < total; i++) {
        if (i != ignorar && strcmp(v[i].cpf, cpf) == 0) return 1;
    }
    return 0;
}

/* Le e valida o CPF (formato, digitos e duplicidade). Resultado em 'destino'.
   Retorna LEITURA_OK, LEITURA_VAZIA (ENTER, se 'opcional') ou LEITURA_CANCELADA (0). */
static int pedirCpf(const Cliente *v, int total, int ignorar, int opcional, char *destino) {
    char buffer[64], digitos[TAM_CPF];
    while (1) {
        printf(opcional ? "Novo CPF (ENTER mantem): " : "CPF (ex: 529.982.247-25): ");
        lerLinha(buffer, sizeof(buffer));
        removerEspacos(buffer);
        if (strcmp(buffer, "0") == 0) return LEITURA_CANCELADA;
        if (buffer[0] == '\0') {
            if (opcional) return LEITURA_VAZIA;
            printf(COR_VERMELHO "Este campo e obrigatorio.\n" COR_RESET);
            continue;
        }
        apenasDigitos(buffer, digitos, sizeof(digitos));
        if (!validarCPF(digitos)) {
            printf(COR_VERMELHO "CPF invalido! Digite um CPF real com 11 digitos (ou 0 para cancelar).\n" COR_RESET);
            continue;
        }
        if (cpfJaCadastrado(v, total, digitos, ignorar)) {
            printf(COR_VERMELHO "Ja existe um cliente com este CPF.\n" COR_RESET);
            continue;
        }
        snprintf(destino, TAM_CPF, "%s", digitos);
        return LEITURA_OK;
    }
}

/* ==========================================================
   Exibicao
   ========================================================== */

static void exibirCliente(const Cliente *c) {
    char cpf[TAM_CPF];
    formatarCPF(c->cpf, cpf);
    printf(COR_CIANO COR_NEGRITO "[%04d] %s\n" COR_RESET, c->codigo, c->nome);
    printf("       CPF: %s | Tel: %s\n", cpf, c->telefone);
    printf("       E-mail: %s | Cidade: %s\n", c->email, c->cidade);
}

/* ==========================================================
   Menu
   ========================================================== */

void menuClientes(void) {
    int opcao;

    do {
        imprimirCabecalho("MODULO DE CLIENTES", COR_CIANO);
        printf(COR_CIANO "\n  1" COR_RESET " - Cadastrar cliente\n");
        printf(COR_CIANO "  2" COR_RESET " - Listar clientes\n");
        printf(COR_CIANO "  3" COR_RESET " - Buscar cliente (codigo / nome / CPF)\n");
        printf(COR_CIANO "  4" COR_RESET " - Alterar cliente\n");
        printf(COR_CIANO "  5" COR_RESET " - Excluir cliente\n");
        printf(COR_VERMELHO "  0" COR_RESET " - Voltar ao menu principal\n");
        imprimirLinha(COR_CIANO);

        opcao = lerInteiro(COR_NEGRITO ">> Escolha uma opcao: " COR_RESET);

        switch (opcao) {
            case 1: cadastrarCliente(); pausar(); break;
            case 2: listarClientes();   pausar(); break;
            case 3: buscarCliente();    pausar(); break;
            case 4: alterarCliente();   pausar(); break;
            case 5: excluirCliente();   pausar(); break;
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

void cadastrarCliente(void) {
    imprimirCabecalho("CADASTRAR CLIENTE", COR_CIANO);
    printf("\n");
    avisoCancelar();

    int total;
    Cliente *v = carregarClientes(&total);
    if (v == NULL) {
        printf(COR_VERMELHO "Erro ao ler o arquivo de clientes.\n" COR_RESET);
        return;
    }

    Cliente novo;
    memset(&novo, 0, sizeof(novo));

    /* Codigo: positivo e sem duplicidade (regra de negocio). */
    while (1) {
        novo.codigo = lerCodigo("Codigo (0 cancela): ");
        if (novo.codigo == 0) {
            printf(COR_AMARELO "Cadastro cancelado.\n" COR_RESET);
            free(v);
            return;
        }
        if (buscaBinariaCliente(v, 0, total - 1, novo.codigo) == -1) break;
        printf(COR_VERMELHO "Ja existe um cliente com o codigo %d.\n" COR_RESET, novo.codigo);
    }

    /* Os campos sao lidos em sequencia. O operador && interrompe a cadeia
       assim que algum campo devolve LEITURA_CANCELADA (usuario digitou 0). */
    int ok = lerTextoObrigatorio("Nome: ", novo.nome, TAM_NOME) != LEITURA_CANCELADA
          && pedirCpf(v, total, -1, 0, novo.cpf) != LEITURA_CANCELADA
          && lerCampoValidado("Telefone (ex: (75) 99999-1234): ", novo.telefone, TAM_TELEFONE,
                              validarTelefone, "Telefone invalido! Use 8 a 13 digitos.", 0) != LEITURA_CANCELADA
          && lerCampoValidado("E-mail: ", novo.email, TAM_EMAIL,
                              validarEmail, "E-mail invalido! Ex: nome@dominio.com", 0) != LEITURA_CANCELADA
          && lerTextoObrigatorio("Cidade: ", novo.cidade, TAM_CIDADE) != LEITURA_CANCELADA;
    if (!ok) {
        printf(COR_AMARELO "\nCadastro cancelado. Nada foi salvo.\n" COR_RESET);
        free(v);
        return;
    }

    /* Aumenta o vetor em 1 posicao e acrescenta o novo cliente. */
    Cliente *aux = (Cliente *)realloc(v, (size_t)(total + 1) * sizeof(Cliente));
    if (aux == NULL) {
        printf(COR_VERMELHO "Memoria insuficiente.\n" COR_RESET);
        free(v);
        return;
    }
    v = aux;
    v[total] = novo;
    total++;

    if (salvarClientes(v, total)) {
        printf(COR_VERDE "\nCliente cadastrado com sucesso!\n" COR_RESET);
    } else {
        printf(COR_VERMELHO "\nErro ao gravar o arquivo de clientes.\n" COR_RESET);
    }
    free(v);
}

void listarClientes(void) {
    imprimirCabecalho("LISTAR CLIENTES", COR_CIANO);

    int total;
    Cliente *v = carregarClientes(&total);
    if (v == NULL) {
        printf(COR_VERMELHO "Erro ao ler o arquivo de clientes.\n" COR_RESET);
        return;
    }
    if (total == 0) {
        printf(COR_AMARELO "\nNenhum cliente cadastrado.\n" COR_RESET);
    } else {
        printf("\n");
        for (int i = 0; i < total; i++) {
            exibirCliente(&v[i]);
            printf("\n");
        }
        printf(COR_VERDE "Total de clientes: %d\n" COR_RESET, total);
    }
    free(v);
}

void buscarCliente(void) {
    imprimirCabecalho("BUSCAR CLIENTE", COR_CIANO);

    int total;
    Cliente *v = carregarClientes(&total);
    if (v == NULL) {
        printf(COR_VERMELHO "Erro ao ler o arquivo de clientes.\n" COR_RESET);
        return;
    }

    printf("\n  1 - Por codigo\n  2 - Por nome (parte do nome)\n  3 - Por CPF\n  0 - Cancelar\n");
    int tipo = lerInteiro(">> Buscar por: ");
    if (tipo == 0) {
        free(v);
        return;
    }
    int achados = 0;

    if (tipo == 1) {
        int cod = lerCodigo("Codigo (0 cancela): ");
        if (cod == 0) { free(v); return; }
        int idx = buscaBinariaCliente(v, 0, total - 1, cod);   /* recursiva */
        if (idx >= 0) {
            printf("\n");
            exibirCliente(&v[idx]);
            achados = 1;
        }
    } else if (tipo == 2) {
        char trecho[TAM_NOME];
        if (lerTextoObrigatorio("Nome ou parte (0 cancela): ", trecho, TAM_NOME) == LEITURA_CANCELADA) {
            free(v);
            return;
        }
        printf("\n");
        for (int i = 0; i < total; i++) {
            if (contemIgnorandoCaso(v[i].nome, trecho)) {
                exibirCliente(&v[i]);
                printf("\n");
                achados++;
            }
        }
    } else if (tipo == 3) {
        char entrada[64], digitos[TAM_CPF];
        if (lerTextoObrigatorio("CPF (0 cancela): ", entrada, sizeof(entrada)) == LEITURA_CANCELADA) {
            free(v);
            return;
        }
        apenasDigitos(entrada, digitos, sizeof(digitos));
        for (int i = 0; i < total; i++) {
            if (strcmp(v[i].cpf, digitos) == 0) {
                printf("\n");
                exibirCliente(&v[i]);
                achados = 1;
                break;
            }
        }
    } else {
        printf(COR_VERMELHO "\nOpcao invalida.\n" COR_RESET);
        free(v);
        return;
    }

    if (achados == 0) {
        printf(COR_AMARELO "\nNenhum cliente encontrado.\n" COR_RESET);
    } else if (tipo == 2) {
        printf(COR_VERDE "Clientes encontrados: %d\n" COR_RESET, achados);
    }
    free(v);
}

void alterarCliente(void) {
    imprimirCabecalho("ALTERAR CLIENTE", COR_CIANO);

    int total;
    Cliente *v = carregarClientes(&total);
    if (v == NULL) {
        printf(COR_VERMELHO "Erro ao ler o arquivo de clientes.\n" COR_RESET);
        return;
    }

    int cod = lerCodigo("Codigo do cliente a alterar (0 cancela): ");
    if (cod == 0) { free(v); return; }

    int idx = buscaBinariaCliente(v, 0, total - 1, cod);
    if (idx < 0) {
        printf(COR_VERMELHO "\nCliente nao encontrado.\n" COR_RESET);
        free(v);
        return;
    }

    Cliente *c = &v[idx];   /* ponteiro para o registro dentro do vetor */
    printf("\nDados atuais:\n");
    exibirCliente(c);
    printf(COR_AMARELO "\nENTER mantem o valor atual | 0 cancela a alteracao.\n\n" COR_RESET);

    /* Os campos sao editados direto no registro (via ponteiro c). Se o usuario
       cancelar no meio, o vetor e descartado SEM gravar: o arquivo nao muda. */
    int ok = lerTextoOpcional("Novo nome: ", c->nome, TAM_NOME) != LEITURA_CANCELADA
          && pedirCpf(v, total, idx, 1, c->cpf) != LEITURA_CANCELADA
          && lerCampoValidado("Novo telefone: ", c->telefone, TAM_TELEFONE,
                              validarTelefone, "Telefone invalido! Use 8 a 13 digitos.", 1) != LEITURA_CANCELADA
          && lerCampoValidado("Novo e-mail: ", c->email, TAM_EMAIL,
                              validarEmail, "E-mail invalido! Ex: nome@dominio.com", 1) != LEITURA_CANCELADA
          && lerTextoOpcional("Nova cidade: ", c->cidade, TAM_CIDADE) != LEITURA_CANCELADA;
    if (!ok) {
        printf(COR_AMARELO "\nAlteracao cancelada. Nada foi salvo.\n" COR_RESET);
        free(v);
        return;
    }

    if (salvarClientes(v, total)) {
        printf(COR_VERDE "\nCliente alterado com sucesso!\n" COR_RESET);
    } else {
        printf(COR_VERMELHO "\nErro ao gravar o arquivo de clientes.\n" COR_RESET);
    }
    free(v);
}

void excluirCliente(void) {
    imprimirCabecalho("EXCLUIR CLIENTE", COR_CIANO);

    int total;
    Cliente *v = carregarClientes(&total);
    if (v == NULL) {
        printf(COR_VERMELHO "Erro ao ler o arquivo de clientes.\n" COR_RESET);
        return;
    }

    int cod = lerCodigo("Codigo do cliente a excluir (0 cancela): ");
    if (cod == 0) { free(v); return; }

    int idx = buscaBinariaCliente(v, 0, total - 1, cod);
    if (idx < 0) {
        printf(COR_VERMELHO "\nCliente nao encontrado.\n" COR_RESET);
        free(v);
        return;
    }

    printf("\n");
    exibirCliente(&v[idx]);

    /* Integridade: nao apaga quem ja tem vendas registradas. */
    if (clienteTemVendas(cod)) {
        printf(COR_VERMELHO "\nExclusao negada: este cliente possui vendas registradas.\n" COR_RESET);
        free(v);
        return;
    }

    if (!confirmar("\nConfirma a exclusao? (S/N): ")) {
        printf(COR_AMARELO "Exclusao cancelada.\n" COR_RESET);
        free(v);
        return;
    }

    /* Desloca os registros seguintes uma posicao para tras. */
    memmove(&v[idx], &v[idx + 1], (size_t)(total - idx - 1) * sizeof(Cliente));
    total--;

    if (salvarClientes(v, total)) {
        printf(COR_VERDE "\nCliente excluido com sucesso!\n" COR_RESET);
    } else {
        printf(COR_VERMELHO "\nErro ao gravar o arquivo de clientes.\n" COR_RESET);
    }
    free(v);
}
