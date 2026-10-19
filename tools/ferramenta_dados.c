#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <time.h>
#include "../include/cliente.h"
#include "../include/produto.h"
#include "../include/venda.h"
#include "../include/utils.h"
#include "../include/arquivo.h"

#ifdef _WIN32
#include <direct.h>
#define CRIAR_PASTA(d) _mkdir(d)
#else
#include <sys/stat.h>
#define CRIAR_PASTA(d) mkdir(d, 0755)
#endif

/* ==========================================================
   ferramenta_dados.c
   Programa AUXILIAR (separado do sistema Arcus) para facilitar testes:

     ferramenta_dados popular [clientes produtos vendas] [--sim]
         Gera dados de exemplo (clientes, produtos e vendas) e grava na
         pasta data (arquivos .dat), substituindo os dados atuais.
         Padrao: 20 clientes, 25 produtos e 60 vendas.

     ferramenta_dados exportar
         Converte os arquivos binarios .dat em planilhas CSV legiveis
         (pasta data/export), que abrem no Excel ou no Bloco de Notas.

     ferramenta_dados limpar [--sim]
         Apaga todos os dados (os arquivos .dat da pasta data).

   Reaproveita as mesmas structs e funcoes do sistema (cliente.c,
   produto.c, venda.c, arquivo.c), entao os arquivos gerados sao
   identicos aos que o programa principal cria.
   ========================================================== */

#define DIR_EXPORT "data/export"

/* ---------- Gerador pseudoaleatorio proprio ----------
   Usa uma formula fixa (congruencial linear) em vez de rand(), para a
   MESMA sequencia de dados em qualquer computador/compilador. */
static uint32_t semente = 20261018u;

static unsigned int sorteio(void) {
    semente = semente * 1103515245u + 12345u;
    return (semente >> 16) & 0x7FFFu;
}

static int entre(int min, int max) {
    return min + (int)(sorteio() % (unsigned int)(max - min + 1));
}

/* Indice com tendencia aos primeiros da lista (os "populares"): sorteia
   dois e fica com o menor. Faz alguns produtos/clientes aparecerem mais. */
static int tendencioso(int n) {
    int a = entre(0, n - 1), b = entre(0, n - 1);
    return a < b ? a : b;
}

/* ---------- Catalogos de exemplo ---------- */
static const char *PRIMEIROS[] = {
    "Maria", "Joao", "Ana", "Carlos", "Fernanda", "Pedro", "Juliana", "Lucas", "Mariana", "Rafael",
    "Camila", "Bruno", "Patricia", "Gabriel", "Aline", "Diego", "Beatriz", "Thiago", "Larissa", "Felipe"
};
static const char *SOBRENOMES[] = {
    "Silva", "Souza", "Santos", "Oliveira", "Pereira", "Lima", "Costa", "Ribeiro",
    "Almeida", "Carvalho", "Gomes", "Martins", "Araujo", "Barbosa", "Rocha"
};
static const char *CIDADES[] = {
    "Santo Amaro", "Cachoeira", "Sao Felix", "Cruz das Almas",
    "Salvador", "Feira de Santana", "Santo Antonio de Jesus", "Muritiba"
};
static const char *DOMINIOS[] = { "email.com", "mail.com", "exemplo.com.br" };

typedef struct {
    const char *nome;
    const char *categoria;
    float       preco;
    int         estoqueInicial;
} ItemCatalogo;

static const ItemCatalogo CATALOGO[] = {
    { "Mouse Sem Fio",            "Perifericos",   49.90f, 60 },
    { "Teclado Mecanico",         "Perifericos",  250.00f, 25 },
    { "Monitor 24",               "Video",        899.90f, 15 },
    { "Cabo HDMI 2m",             "Acessorios",    19.90f, 80 },
    { "Headset Gamer",            "Audio",        189.90f, 30 },
    { "Webcam Full HD",           "Perifericos",  159.90f, 20 },
    { "Pen Drive 64GB",           "Armazenamento", 39.90f, 70 },
    { "SSD 480GB",                "Armazenamento",279.90f, 22 },
    { "HD Externo 1TB",           "Armazenamento",349.90f, 18 },
    { "Mousepad Grande",          "Acessorios",    29.90f, 50 },
    { "Carregador USB-C",         "Acessorios",    59.90f, 45 },
    { "Fone Bluetooth",           "Audio",        129.90f, 35 },
    { "Caixa de Som",             "Audio",         99.90f, 28 },
    { "Notebook 14",              "Informatica", 3299.00f,  8 },
    { "Roteador Wi-Fi",           "Redes",        219.90f, 16 },
    { "Cabo de Rede 5m",          "Redes",         24.90f, 60 },
    { "Impressora Jato de Tinta", "Informatica",  649.00f,  6 },
    { "Cartucho Preto",           "Suprimentos",   79.90f, 40 },
    { "Resma Papel A4",           "Papelaria",     27.50f, 90 },
    { "Caneta Esferografica Cx",  "Papelaria",     18.00f, 70 },
    { "Estabilizador 600VA",      "Informatica",  189.00f, 12 },
    { "Suporte para Notebook",    "Acessorios",    69.90f, 25 },
    { "Cadeira de Escritorio",    "Moveis",       749.90f,  7 },
    { "Mesa para Computador",     "Moveis",       499.90f,  5 },
    { "Pilha AA Pacote 4",        "Acessorios",    14.90f,100 }
};

#define QTD_PRIMEIROS   ((int)(sizeof(PRIMEIROS)  / sizeof(PRIMEIROS[0])))
#define QTD_SOBRENOMES  ((int)(sizeof(SOBRENOMES) / sizeof(SOBRENOMES[0])))
#define QTD_CIDADES     ((int)(sizeof(CIDADES)    / sizeof(CIDADES[0])))
#define QTD_DOMINIOS    ((int)(sizeof(DOMINIOS)   / sizeof(DOMINIOS[0])))
#define QTD_CATALOGO    ((int)(sizeof(CATALOGO)   / sizeof(CATALOGO[0])))

#define MAX_CLIENTES (QTD_PRIMEIROS * QTD_SOBRENOMES)   /* 300 combinacoes de nome */
#define MAX_VENDAS   2000

/* ---------- Geracao de CPF valido ---------- */

/* Digito verificador (mesma regra do sistema): pesos n+1 ... 2, resto de 11. */
static int digitoVerificador(const int *d, int n) {
    int soma = 0;
    for (int i = 0; i < n; i++) soma += d[i] * (n + 1 - i);
    int resto = soma % 11;
    return (resto < 2) ? 0 : 11 - resto;
}

static void gerarCpf(char *destino, const Cliente *ja, int quantos) {
    int repetido;
    do {
        int d[11];
        for (int i = 0; i < 9; i++) d[i] = entre(0, 9);
        d[9]  = digitoVerificador(d, 9);
        d[10] = digitoVerificador(d, 10);
        for (int i = 0; i < 11; i++) destino[i] = (char)('0' + d[i]);
        destino[11] = '\0';

        repetido = !validarCPF(destino);              /* recusa 111.111.111-11 etc. */
        for (int i = 0; i < quantos && !repetido; i++) {
            if (strcmp(ja[i].cpf, destino) == 0) repetido = 1;
        }
    } while (repetido);
}

static void minusculas(char *texto) {
    for (; *texto != '\0'; texto++) {
        if (*texto >= 'A' && *texto <= 'Z') *texto = (char)(*texto + 32);
    }
}

/* ==========================================================
   POPULAR
   ========================================================== */

static int popular(int nClientes, int nProdutos, int nVendas) {
    Cliente *clientes = (Cliente *)calloc((size_t)nClientes, sizeof(Cliente));
    Produto *produtos = (Produto *)calloc((size_t)nProdutos, sizeof(Produto));
    Venda   *vendas   = (Venda *)calloc((size_t)nVendas, sizeof(Venda));
    if (clientes == NULL || produtos == NULL || vendas == NULL) {
        printf("Memoria insuficiente.\n");
        free(clientes); free(produtos); free(vendas);
        return 0;
    }

    /* ---- Clientes ---- */
    for (int i = 0; i < nClientes; i++) {
        const char *primeiro  = PRIMEIROS[(i * 7) % QTD_PRIMEIROS];
        const char *sobrenome = SOBRENOMES[(i * 3 + i / QTD_PRIMEIROS) % QTD_SOBRENOMES];
        char p[TAM_NOME], s[TAM_NOME];

        clientes[i].codigo = i + 1;
        snprintf(clientes[i].nome, TAM_NOME, "%s %s", primeiro, sobrenome);
        gerarCpf(clientes[i].cpf, clientes, i);
        snprintf(clientes[i].telefone, TAM_TELEFONE, "(75) 9%04d-%04d", entre(8000, 9999), entre(0, 9999));
        snprintf(p, sizeof p, "%s", primeiro);
        snprintf(s, sizeof s, "%s", sobrenome);
        minusculas(p);
        minusculas(s);
        snprintf(clientes[i].email, TAM_EMAIL, "%.15s.%.15s%d@%.15s", p, s, i + 1, DOMINIOS[i % QTD_DOMINIOS]);
        snprintf(clientes[i].cidade, TAM_CIDADE, "%s", CIDADES[entre(0, QTD_CIDADES - 1)]);
    }

    /* ---- Produtos (estoque inicial do catalogo) ---- */
    for (int i = 0; i < nProdutos; i++) {
        produtos[i].codigo = 101 + i;
        snprintf(produtos[i].nome, TAM_NOME_PROD, "%s", CATALOGO[i].nome);
        snprintf(produtos[i].categoria, TAM_CATEGORIA, "%s", CATALOGO[i].categoria);
        produtos[i].preco = CATALOGO[i].preco;
        produtos[i].quantidadeEstoque = CATALOGO[i].estoqueInicial;
    }

    /* ---- Vendas: respeitam o estoque e vao baixando as quantidades ---- */
    time_t agora = time(NULL);
    int geradas = 0, tentativas = 0;

    while (geradas < nVendas && tentativas < nVendas * 20) {
        tentativas++;
        Venda v;
        memset(&v, 0, sizeof(v));

        int alvo = entre(1, 4);                       /* ate 4 itens por venda */
        for (int k = 0; k < alvo; k++) {
            int idx = tendencioso(nProdutos);
            if (produtos[idx].quantidadeEstoque <= 0) continue;

            int repetido = 0;
            for (int j = 0; j < v.numItens; j++) {
                if (v.itens[j].codigoProduto == produtos[idx].codigo) repetido = 1;
            }
            if (repetido) continue;

            int qtd = entre(1, 3);
            if (qtd > produtos[idx].quantidadeEstoque) qtd = produtos[idx].quantidadeEstoque;

            ItemVenda *it = &v.itens[v.numItens++];
            it->codigoProduto = produtos[idx].codigo;
            snprintf(it->nomeProduto, TAM_NOME_PROD, "%s", produtos[idx].nome);
            it->quantidade = qtd;
            it->precoUnitario = produtos[idx].preco;
            it->subtotal = qtd * produtos[idx].preco;
            produtos[idx].quantidadeEstoque -= qtd;
        }
        if (v.numItens == 0) continue;

        v.codigo = geradas + 1;
        v.codigoCliente = clientes[tendencioso(nClientes)].codigo;
        v.valorTotal = totalItensRecursivo(v.itens, v.numItens);

        /* Datas crescentes: a primeira venda e a mais antiga (ate 90 dias atras). */
        long diasAtras = (long)(nVendas - 1 - geradas) * 90 / nVendas;
        time_t quando = agora - (time_t)diasAtras * 86400;
        strftime(v.data, TAM_DATA, "%d/%m/%Y", localtime(&quando));

        vendas[geradas++] = v;
    }

    /* ---- Gravacao (mesmas funcoes do sistema) ---- */
    int ok = salvarClientes(clientes, nClientes)
          && salvarProdutos(produtos, nProdutos)
          && salvarRegistros(ARQ_VENDAS, vendas, sizeof(Venda), geradas);

    if (ok) {
        double total = 0.0;
        int baixos = 0, esgotados = 0;
        for (int i = 0; i < geradas; i++) total += vendas[i].valorTotal;
        for (int i = 0; i < nProdutos; i++) {
            if (produtos[i].quantidadeEstoque <= LIMITE_ESTOQUE_BAIXO) baixos++;
            if (produtos[i].quantidadeEstoque == 0) esgotados++;
        }
        printf("\nDados de exemplo gravados em data/ com sucesso!\n");
        printf("  Clientes : %d\n", nClientes);
        printf("  Produtos : %d (%d com estoque baixo, %d esgotados)\n", nProdutos, baixos, esgotados);
        printf("  Vendas   : %d (total vendido: R$ %.2f)\n", geradas, total);
        printf("\nAgora abra o sistema Arcus e confira os relatorios e as estatisticas.\n");
    } else {
        printf("\nErro ao gravar os arquivos em data/.\n");
    }

    free(clientes); free(produtos); free(vendas);
    return ok;
}

/* ==========================================================
   EXPORTAR (CSV)
   Separador ";" e virgula decimal: abre direto no Excel em portugues.
   ========================================================== */

static void csvTexto(FILE *f, const char *s) {
    for (; *s != '\0'; s++) fputc(*s == ';' ? ',' : *s, f);
}

static void csvNumero(FILE *f, double v) {
    char b[48];
    snprintf(b, sizeof b, "%.2f", v);
    for (char *p = b; *p != '\0'; p++) if (*p == '.') *p = ',';
    fputs(b, f);
}

static FILE *abrirCsv(const char *nome) {
    char caminho[128];
    snprintf(caminho, sizeof caminho, "%s/%s", DIR_EXPORT, nome);
    FILE *f = fopen(caminho, "w");
    if (f == NULL) printf("Nao foi possivel criar %s\n", caminho);
    return f;
}

static int exportar(void) {
    int nC, nP, nV;
    Cliente *clientes = carregarClientes(&nC);
    Produto *produtos = carregarProdutos(&nP);
    Venda   *vendas   = carregarVendas(&nV);
    int ok = 0;

    if (clientes == NULL || produtos == NULL || vendas == NULL) {
        printf("Erro ao ler os arquivos de dados.\n");
    } else {
        CRIAR_PASTA(DIR_DADOS);
        CRIAR_PASTA(DIR_EXPORT);

        FILE *fc = abrirCsv("clientes.csv");
        FILE *fp = abrirCsv("produtos.csv");
        FILE *fv = abrirCsv("vendas.csv");
        FILE *fi = abrirCsv("itens_venda.csv");

        if (fc && fp && fv && fi) {
            fputs("codigo;nome;cpf;telefone;email;cidade\n", fc);
            for (int i = 0; i < nC; i++) {
                const char *d = clientes[i].cpf;
                fprintf(fc, "%d;", clientes[i].codigo);
                csvTexto(fc, clientes[i].nome);
                fprintf(fc, ";%.3s.%.3s.%.3s-%.2s;", d, d + 3, d + 6, d + 9);
                csvTexto(fc, clientes[i].telefone); fputc(';', fc);
                csvTexto(fc, clientes[i].email);    fputc(';', fc);
                csvTexto(fc, clientes[i].cidade);   fputc('\n', fc);
            }

            fputs("codigo;nome;categoria;preco;estoque;valor_em_estoque\n", fp);
            for (int i = 0; i < nP; i++) {
                fprintf(fp, "%d;", produtos[i].codigo);
                csvTexto(fp, produtos[i].nome);      fputc(';', fp);
                csvTexto(fp, produtos[i].categoria); fputc(';', fp);
                csvNumero(fp, produtos[i].preco);
                fprintf(fp, ";%d;", produtos[i].quantidadeEstoque);
                csvNumero(fp, (double)produtos[i].preco * produtos[i].quantidadeEstoque);
                fputc('\n', fp);
            }

            fputs("venda;data;codigo_cliente;nome_cliente;qtd_itens;total\n", fv);
            fputs("venda;codigo_produto;produto;quantidade;preco_unitario;subtotal\n", fi);
            for (int i = 0; i < nV; i++) {
                int idx = buscaBinariaCliente(clientes, 0, nC - 1, vendas[i].codigoCliente);
                fprintf(fv, "%d;%s;%d;", vendas[i].codigo, vendas[i].data, vendas[i].codigoCliente);
                csvTexto(fv, idx >= 0 ? clientes[idx].nome : "(removido)");
                fprintf(fv, ";%d;", vendas[i].numItens);
                csvNumero(fv, vendas[i].valorTotal);
                fputc('\n', fv);

                for (int j = 0; j < vendas[i].numItens; j++) {
                    const ItemVenda *it = &vendas[i].itens[j];
                    fprintf(fi, "%d;%d;", vendas[i].codigo, it->codigoProduto);
                    csvTexto(fi, it->nomeProduto);
                    fprintf(fi, ";%d;", it->quantidade);
                    csvNumero(fi, it->precoUnitario); fputc(';', fi);
                    csvNumero(fi, it->subtotal);      fputc('\n', fi);
                }
            }
            ok = 1;
            printf("Arquivos gerados na pasta %s/:\n", DIR_EXPORT);
            printf("  clientes.csv     (%d linhas)\n", nC);
            printf("  produtos.csv     (%d linhas)\n", nP);
            printf("  vendas.csv       (%d linhas)\n", nV);
            printf("  itens_venda.csv  (itens de cada venda)\n");
            printf("\nAbra com o Excel (clique duplo) ou com o Bloco de Notas.\n");
        }
        if (fc) fclose(fc);
        if (fp) fclose(fp);
        if (fv) fclose(fv);
        if (fi) fclose(fi);
    }
    free(clientes); free(produtos); free(vendas);
    return ok;
}

/* ==========================================================
   LIMPAR
   ========================================================== */

static void limpar(void) {
    const char *arquivos[] = { ARQ_CLIENTES, ARQ_PRODUTOS, ARQ_VENDAS };
    int apagados = 0;
    for (int i = 0; i < 3; i++) {
        if (remove(arquivos[i]) == 0) apagados++;
    }
    printf("%d arquivo(s) de dados apagado(s). O sistema comecara vazio.\n", apagados);
}

/* ==========================================================
   main
   ========================================================== */

static void uso(void) {
    printf("Uso:\n");
    printf("  ferramenta_dados popular [clientes produtos vendas] [--sim]\n");
    printf("        gera dados de exemplo (padrao: 20 clientes, 25 produtos, 60 vendas)\n");
    printf("        limites: clientes ate %d, produtos ate %d, vendas ate %d\n",
           MAX_CLIENTES, QTD_CATALOGO, MAX_VENDAS);
    printf("  ferramenta_dados exportar\n");
    printf("        converte os .dat em planilhas CSV legiveis (data/export/)\n");
    printf("  ferramenta_dados limpar [--sim]\n");
    printf("        apaga todos os dados\n");
    printf("  --sim : nao pergunta confirmacao\n");
}

static int limitar(int v, int min, int max) {
    return v < min ? min : (v > max ? max : v);
}

int main(int argc, char **argv) {
    if (argc < 2) {
        uso();
        return 1;
    }

    int sim = 0;
    int numeros[3] = { 20, 25, 60 };
    int qtdNumeros = 0;
    for (int i = 2; i < argc; i++) {
        if (strcmp(argv[i], "--sim") == 0) sim = 1;
        else if (qtdNumeros < 3) numeros[qtdNumeros++] = atoi(argv[i]);
    }

    if (strcmp(argv[1], "popular") == 0) {
        int nC = limitar(numeros[0], 1, MAX_CLIENTES);
        int nP = limitar(numeros[1], 1, QTD_CATALOGO);
        int nV = limitar(numeros[2], 1, MAX_VENDAS);
        printf("Serao gerados: %d clientes, %d produtos e %d vendas.\n", nC, nP, nV);
        printf("ATENCAO: isto SUBSTITUI os dados atuais da pasta data.\n");
        if (!sim && !confirmar("Continuar? (S/N): ")) {
            printf("Operacao cancelada.\n");
            return 0;
        }
        return popular(nC, nP, nV) ? 0 : 1;
    }
    if (strcmp(argv[1], "exportar") == 0) {
        return exportar() ? 0 : 1;
    }
    if (strcmp(argv[1], "limpar") == 0) {
        printf("ATENCAO: isto APAGA todos os clientes, produtos e vendas.\n");
        if (!sim && !confirmar("Continuar? (S/N): ")) {
            printf("Operacao cancelada.\n");
            return 0;
        }
        limpar();
        return 0;
    }

    uso();
    return 1;
}
