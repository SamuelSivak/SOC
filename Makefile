# Kompilátor a príznaky
CC = gcc
CFLAGS = -Wall -Wextra -O2 -I./include
LDFLAGS = -lm

# Adresáre
SRC_DIR = src
INCLUDE_DIR = include
EXAMPLES_DIR = examples
BIN_DIR = bin
OBJ_DIR = obj

# Zdrojové súbory
SRCS = $(wildcard $(SRC_DIR)/*.c)
EXAMPLES = $(wildcard $(EXAMPLES_DIR)/*.c)
OBJS = $(SRCS:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)
EXAMPLE_OBJS = $(EXAMPLES:$(EXAMPLES_DIR)/%.c=$(OBJ_DIR)/%.o)
EXAMPLE_BINS = $(EXAMPLES:$(EXAMPLES_DIR)/%.c=$(BIN_DIR)/%)

# Hlavný cieľ
all: directories $(EXAMPLE_BINS)

# Vytvorenie potrebných adresárov
directories:
	@mkdir -p $(BIN_DIR)
	@mkdir -p $(OBJ_DIR)
	@mkdir -p models

# Pravidlo pre kompiláciu zdrojových súborov
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

# Pravidlo pre kompiláciu príkladov
$(OBJ_DIR)/%.o: $(EXAMPLES_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

# Pravidlo pre linkovanie príkladov
$(BIN_DIR)/%: $(OBJ_DIR)/%.o $(OBJS)
	$(CC) $^ -o $@ $(LDFLAGS)

# Vyčistenie projektu
clean:
	rm -rf $(BIN_DIR)
	rm -rf $(OBJ_DIR)

# Vyčistenie všetkého vrátane modelov
cleanall: clean
	rm -rf models/*

# Phony ciele
.PHONY: all clean cleanall directories

# Závislosti
-include $(OBJS:.o=.d)
-include $(EXAMPLE_OBJS:.o=.d) 
.PHONY: all clean 