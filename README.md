# Arcus — Sistema de Gerenciamento Empresarial

> **Disciplina:** Algoritmos e Programação II  
> **Instituição:** Universidade Federal do Recôncavo da Bahia (UFRB / CETENS) — Bacharelado em Sistemas de Informação (EaD)  
> **Docente:** Prof. Alex Ferreira dos Santos  
> **Discente:** Priscila Silva dos Santos  

---

## 📖 Apresentação e Objetivos

O **Arcus** é um sistema de gerenciamento comercial desenvolvido em **Linguagem C** voltado para a gestão de clientes, produtos e registro de vendas com baixa automática em estoque e geração de estatísticas. 

Este projeto foi desenvolvido como requisito de avaliação da disciplina de **Algoritmos e Programação II**, tendo como objetivo demonstrar a aplicação prática e integrada de conceitos fundamentais da ciência da computação e engenharia de software: **modularização, estruturas de dados heterogêneas, alocação dinâmica de memória, ponteiros, recursividade e persistência de dados binários**.

---

## 🛠️ Fundamentação Teórica e Conceitos de Algoritmos II

Abaixo estão detalhados os principais conceitos de programação estruturada e avançada em C utilizados durante o desenvolvimento do sistema:

### 1. Estruturas de Dados Heterogêneas (`struct`)
Diferente de um *array* (vetor), que armazena múltiplos elementos do mesmo tipo de dado, uma `struct` permite agregar diferentes tipos de dados sob uma mesma unidade lógica.
* **Aplicação no Projeto:** Definição dos tipos `Cliente`, `Produto`, `ItemVenda` e `Venda`.
```c
typedef struct {
    int   codigo;
    char  nome[60];
    char  categoria[40];
    float preco;
    int   quantidadeEstoque;
} Produto;
```

### 2. Funções e Modularização
A modularização divide o código em arquivos de cabeçalho (`.h`) e arquivos de implementação (`.c`). Isso garante a separação de responsabilidades e facilita a manutenção.
* **Header Files (`include/*.h`):** Contêm as definições das estruturas, constantes e protótipos das funções.
* **Implementation Files (`src/*.c`):** Contêm o código executável e a lógica das operações.

### 3. Ponteiros e Passagem por Referência
Ponteiros armazenam endereços de memória. Eles são fundamentais para passar dados para funções por referência (permitindo alteração direta do valor original) e evitar a cópia custosa de dados na memória.
* **Exemplo de uso:** A função `pedirPreco` recebe um ponteiro para modificar o valor do preço diretamente no registro do produto:
```c
static int pedirPreco(const char *mensagem, float *destino) {
    // *destino altera o valor do campo original no endereço informado
}
```

### 4. Alocação Dinâmica de Memória (`malloc`, `realloc`, `free`)
A alocação dinâmica permite gerenciar a memória do computador em tempo de execução (*heap*), ajustando o tamanho de vetores de acordo com a quantidade real de dados gravados.
* **Aplicação:** Carregamento de registros dos arquivos `.dat` para a memória principal (`carregarRegistros`) e inclusão progressiva de novos itens via `realloc`. A liberação da memória alocada é sempre realizada com `free()`.

### 5. Recursividade
A recursividade ocorre quando uma função chama a si mesma até atingir um caso base.
* **Busca Binária Recursiva (`buscaBinariaCliente` / `buscaBinariaProduto`):** Localiza registros em complexidade $O(\log n)$.
* **Cálculos Acumulados (`valorEstoqueRecursivo`, `totalItensRecursivo`):** Demonstra o cálculo do valor total somando o elemento $n-1$ ao resultado dos elementos anteriores.

### 6. Persistência de Dados em Arquivos Binários
Para manter os dados salvos após o encerramento do programa, o sistema faz uso de arquivos binários (`data/*.dat`) acessados via `fopen`, `fread`, `fwrite` e `fclose`. A gravação é feita gravando o bloco de memória da `struct` diretamente no disco.

---

## 📁 Estrutura do Projeto

```text
sistema_gerenciamento_empresarial/
├── Makefile                # Roteiro automatizado de compilação
├── README.md               # Documentação principal
├── RELATORIO.md            # Relatório técnico e decisões de projeto
├── GUIA_APRESENTACAO.md    # Guia para apresentação e perguntas/respostas
├── include/                # Declarações e protótipos (.h)
│   ├── arquivo.h
│   ├── cliente.h
│   ├── produto.h
│   ├── relatorio.h
│   ├── utils.h
│   └── venda.h
├── src/                    # Implementações em C (.c)
│   ├── arquivo.c
│   ├── cliente.c
│   ├── main.c
│   ├── produto.c
│   ├── relatorio.c
│   ├── utils.c
│   └── venda.c
├── tools/                  # Ferramentas auxiliares
│   └── ferramenta_dados.c  # Gerador de dados de teste e exportador CSV
└── data/                   # Arquivos de dados binários (.dat)
```

---

## 🚀 Como Clonar e Executar o Projeto

### Pré-requisitos
* Compilador C (como `gcc` ou `clang`).
* Utilitário `make` (opcional, mas recomendado).
* Git instalado na sua máquina.

### 1. Clonando o Repositório
Abra o terminal do seu sistema operacional e execute:

```bash
git clone https://github.com/Priscila-Santos/arcus_sistema_de_gestao_empresarial
cd sistema_gerenciamento_empresarial
```

---

### 2. Compilação e Execução

#### Opção A: Utilizando `make` (Linux, macOS ou MSYS2 no Windows)

Compilar o projeto:
```bash
make
```

Compilar e executar diretamente:
```bash
make run
```

Gerar dados aleatórios de teste para relatórios:
```bash
make popular
```

Exportar dados para tabelas `.csv`:
```bash
make exportar
```

Limpar arquivos binários e objetos compilados (`.o`, `.exe`):
```bash
make clean
```

---

#### Opção B: Compilação Manual via GCC (Windows PowerShell)

Caso não possua o utilitário `make` instalado:

**Compilar:**
```powershell
gcc src/main.c src/utils.c src/arquivo.c src/cliente.c src/produto.c src/venda.c src/relatorio.c -Iinclude -Wall -Wextra -o bin/sistema.exe
```

**Executar:**
```powershell
.\bin\sistema.exe
```

---

## 📋 Licença e Autoria

Trabalho prático desenvolvido para a disciplina de **Algoritmos e Programação II**. Sinta-se à vontade para utilizar o código para fins de estudo e aprendizado.