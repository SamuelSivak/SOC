#===============================================================================
# MARKNET - MAKEFILE KONFIGURÁCIA PRE C NEURÓNOVÚ SIEŤ
#===============================================================================
#
# GNU Make konfiguračný súbor pre kompiláciu MarkNET neurónovej siete.
# Obsahuje všetky pravidlá pre build systém, dependency management
# a správu objektových súborov pre MNIST digit recognition projekt.
#
# Komponenty:
# - C neurónová sieť implementation
# - Trénovacie príklady (300, 1500, full dataset)
# - Testing framework a benchmarky
# - Model serialization/deserialization
#
# Použitie:
#   make         - kompiluje všetky príklady
#   make clean   - vymaže build artefakty
#   make cleanall - vymaže build + modely
#
# Autor: Samuel Sivák
# Verzia: 1.0.0
#===============================================================================

#==============================================================================
# KOMPILÁTOR A BUILD NASTAVENIA
#==============================================================================

# GCC kompilátor s optimalizáciou a debugging informáciami
CC = gcc

# Compiler flags pre production build
CFLAGS = -Wall -Wextra -O2 -I./include
# -Wall:    všetky štandardné warnings
# -Wextra:  extra warnings pre kvalitu kódu
# -O2:      optimalizácia pre rýchlosť
# -I:       include path pre header súbory

# Linker flags pre matematické operácie
LDFLAGS = -lm
# -lm:      linkuje matematickú knižnicu (math.h)

#==============================================================================
# ADRESÁROVÁ ŠTRUKTÚRA PROJEKTU
#==============================================================================

# Zdrojové súbory neurónovej siete
SRC_DIR = src

# Header súbory (.h definície)
INCLUDE_DIR = include

# Trénovacie a testovacie príklady
EXAMPLES_DIR = examples

# Výstupné binárne súbory
BIN_DIR = bin

# Objektové súbory (.o intermediate files)
OBJ_DIR = obj

#==============================================================================
# AUTOMATICKÁ DETEKCIA SÚBOROV
#==============================================================================

# Všetky C zdrojové súbory neurónovej siete
SRCS = $(wildcard $(SRC_DIR)/*.c)
# Wildcards: Activation.c, Layer.c, Matrix.c, NeuronNetwork.c, etc.

# Všetky C príklady na kompiláciu
EXAMPLES = $(wildcard $(EXAMPLES_DIR)/*.c)
# Wildcards: mnist_train_300.c, mnist_train_1500.c, test_network.c, etc.

# Objektové súbory odvodené zo zdrojových súborov
OBJS = $(SRCS:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)
# Pattern substitution: src/X.c -> obj/X.o

# Objektové súbory pre príklady
EXAMPLE_OBJS = $(EXAMPLES:$(EXAMPLES_DIR)/%.c=$(OBJ_DIR)/%.o)
# Pattern substitution: examples/X.c -> obj/X.o

# Binárne súbory odvodené z príkladov
EXAMPLE_BINS = $(EXAMPLES:$(EXAMPLES_DIR)/%.c=$(BIN_DIR)/%)
# Pattern substitution: examples/X.c -> bin/X

#==============================================================================
# HLAVNÉ BUILD TARGETY
#==============================================================================

# Predvolený target - kompiluje všetky príklady
all: directories $(EXAMPLE_BINS)

# Vytvorenie potrebných adresárov pre build process
directories:
	@mkdir -p $(BIN_DIR)      # Binárne súbory
	@mkdir -p $(OBJ_DIR)      # Objektové súbory
	@mkdir -p models          # Natrénované modely

#==============================================================================
# KOMPILAČNÉ PRAVIDLÁ
#==============================================================================

# Pravidlo pre kompiláciu zdrojových súborov neurónovej siete
# src/X.c -> obj/X.o
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

# Pravidlo pre kompiláciu príkladov
# examples/X.c -> obj/X.o
$(OBJ_DIR)/%.o: $(EXAMPLES_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

# Pravidlo pre linkovanie príkladov do spustiteľných bináriek
# obj/X.o + všetky OBJS -> bin/X
$(BIN_DIR)/%: $(OBJ_DIR)/%.o $(OBJS)
	$(CC) $^ -o $@ $(LDFLAGS)

#==============================================================================
# UTILITY A CLEANUP TARGETY
#==============================================================================

# Vyčistenie všetkých build artefaktov
clean:
	rm -rf $(BIN_DIR)         # Spustiteľné súbory
	rm -rf $(OBJ_DIR)         # Objektové súbory

# Kompletné vyčistenie vrátane natrénovaných modelov
cleanall: clean
	rm -rf models/*           # Všetky .bin model súbory

#==============================================================================
# MAKE KONFIGURÁCIA A METADATA
#==============================================================================

# Phony targets - nie sú súbory ale príkazy
.PHONY: all clean cleanall directories

# Automatické dependency tracking pre header súbory
# Zahŕňa .d súbory ak existujú (generované GCC -MMD)
-include $(OBJS:.o=.d)
-include $(EXAMPLE_OBJS:.o=.d) 
.PHONY: all clean 