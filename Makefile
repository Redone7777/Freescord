CC      := gcc
CFLAGS  := -g -Wall -Wvla -std=c99 -pthread -D_XOPEN_SOURCE=700 -Iinclude -Iinclude/buffer -Iinclude/list
LDFLAGS := -pthread -Wall

# Dossiers
SRC_DIR := src
INC_DIR := include
BUILD_DIR := build
BIN_DIR := bin

# Cibles exécutables
BIN_SRV := $(BIN_DIR)/srv
BIN_CLT := $(BIN_DIR)/clt
BIN_TEST := $(BIN_DIR)/test_list

# Fichiers sources
SRCS := \
	$(SRC_DIR)/client.c \
	$(SRC_DIR)/serveur.c \
	$(SRC_DIR)/user.c \
	$(SRC_DIR)/utils.c \
	$(INC_DIR)/buffer/buffer.c \
	$(INC_DIR)/list/list.c \
	$(INC_DIR)/list/test_list.c

# Objets (.o) dans build/
OBJS := $(patsubst %.c,$(BUILD_DIR)/%.o,$(SRCS))

# Cibles par défaut
all: $(BIN_SRV) $(BIN_CLT)

# Compilation des exécutables
$(BIN_SRV): $(BUILD_DIR)/src/serveur.o $(BUILD_DIR)/include/list/list.o $(BUILD_DIR)/src/user.o | $(BIN_DIR)
	$(CC) $(LDFLAGS) $^ -o $@

$(BIN_CLT): $(BUILD_DIR)/src/client.o $(BUILD_DIR)/include/buffer/buffer.o $(BUILD_DIR)/src/utils.o | $(BIN_DIR)
	$(CC) $(LDFLAGS) $^ -o $@

$(BIN_TEST): $(BUILD_DIR)/include/list/test_list.o $(BUILD_DIR)/include/list/list.o | $(BIN_DIR)
	$(CC) $(LDFLAGS) $^ -o $@

# Compilation des fichiers objets dans build/
$(BUILD_DIR)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

# Créer le dossier bin s’il n’existe pas
$(BIN_DIR):
	mkdir -p $(BIN_DIR)

test: $(BIN_TEST)
	valgrind --leak-check=full ./$<

clean:
	rm -rf $(BUILD_DIR) $(BIN_DIR)

.PHONY: all clean test
