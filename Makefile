# ==========================================================
# Makefile - Sistema de Gerenciamento Empresarial
#
# O que e: um "roteiro de compilacao". Em vez de digitar o comando longo do
# gcc com todos os arquivos, basta digitar "make". O make tambem so recompila
# o que mudou (compara a data de modificacao dos arquivos).
#
# Uso:
#   make        -> compila o projeto (gera bin/sistema ou bin/sistema.exe)
#   make run    -> compila (se preciso) e executa
#   make clean  -> apaga os arquivos gerados (.o e executavel)
#   make popular      -> gera dados de exemplo para testar (substitui os dados)
#   make exportar     -> converte os .dat em planilhas CSV legiveis
#   make zerar-dados  -> apaga todos os dados
#   (os dados em data/*.dat NAO sao apagados pelo clean)
#
# Cada regra tem o formato:
#   alvo: dependencias
#   <TAB>comando        (a indentacao OBRIGATORIAMENTE e um TAB, nao espacos)
# ==========================================================

# ---- Variaveis ----

# Compilador usado.
CC = gcc

# -Wall -Wextra: mostra avisos de possiveis erros | -Iinclude: pasta onde estao os .h
CFLAGS = -Wall -Wextra -Iinclude

# No Windows o executavel termina em .exe (a variavel de ambiente OS vale "Windows_NT").
ifeq ($(OS),Windows_NT)
    EXE = .exe
endif

# Executavel final.
BIN = bin/sistema$(EXE)

# Todos os .c da pasta src/.
SRC = $(wildcard src/*.c)

# Lista equivalente de .o em obj/ (src/main.c -> obj/main.o).
OBJ = $(patsubst src/%.c, obj/%.o, $(SRC))

# Todos os .h (se algum mudar, tudo e recompilado).
HEADERS = $(wildcard include/*.h)

# Ferramenta auxiliar de dados (popular, exportar, limpar): programa separado.
TOOL = bin/ferramenta_dados$(EXE)
TOOL_SRC = tools/ferramenta_dados.c src/arquivo.c src/cliente.c src/produto.c src/venda.c src/utils.c

# ---- Regras ----

# Alvo padrao (executado quando se digita apenas "make").
all: dirs $(BIN)

# Cria as pastas necessarias, caso nao existam.
dirs:
	@mkdir -p obj bin data

# ETAPA 2 (linkagem): junta todos os .o em um unico executavel.
$(BIN): $(OBJ)
	$(CC) $(OBJ) -o $(BIN)

# ETAPA 1 (compilacao): transforma cada .c em um .o (codigo de maquina ainda "solto").
#   $<  = o arquivo .c    |    $@ = o arquivo .o de saida
obj/%.o: src/%.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

# Compila (se preciso) e executa.
run: all
	./$(BIN)

# ---- Ferramenta de dados (para testar com muitos dados) ----

# Compila a ferramenta (programa separado do sistema).
$(TOOL): $(TOOL_SRC) $(HEADERS)
	$(CC) $(CFLAGS) $(TOOL_SRC) -o $(TOOL)

# make popular    -> gera dados de exemplo (20 clientes, 25 produtos, 60 vendas)
popular: dirs $(TOOL)
	./$(TOOL) popular

# make exportar   -> converte os .dat em planilhas CSV (data/export/)
exportar: dirs $(TOOL)
	./$(TOOL) exportar

# make zerar-dados -> apaga todos os dados
zerar-dados: dirs $(TOOL)
	./$(TOOL) limpar

# Remove arquivos gerados.
clean:
	rm -f obj/*.o bin/sistema bin/sistema.exe bin/ferramenta_dados bin/ferramenta_dados.exe

# Alvos "falsos": nao sao arquivos, sao apenas nomes de comandos.
.PHONY: all dirs run clean popular exportar zerar-dados
