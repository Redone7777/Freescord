CC      := gcc
CFLAGS  := -g -Wall -Wvla -std=c99 -pthread -D_XOPEN_SOURCE=700 -Iinclude -Iinclude/buffer -Iinclude/list
LDFLAGS := -pthread -Wall

# Flags pour GTK
GTK_CFLAGS := $(shell pkg-config --cflags gtk+-3.0)
GTK_LIBS   := $(shell pkg-config --libs gtk+-3.0)

# Dossiers
SRC_DIR := src
INC_DIR := include
BUILD_DIR := build
BIN_DIR := bin

# Binaries
BIN_SRV := $(BIN_DIR)/srv
BIN_CLT := $(BIN_DIR)/clt
BIN_GUI := $(BIN_DIR)/gui
BIN_TEST := $(BIN_DIR)/test_list

# Sources
SRC_SRV := $(SRC_DIR)/serveur.c $(SRC_DIR)/user.c
SRC_CLT := $(SRC_DIR)/client.c $(SRC_DIR)/utils.c
SRC_GUI := $(SRC_DIR)/freescord_gui.c $(SRC_DIR)/utils.c # ⚠️ client.c retiré
SRC_BUFFER := $(INC_DIR)/buffer/buffer.c
SRC_LIST := $(INC_DIR)/list/list.c
SRC_TEST := $(INC_DIR)/list/test_list.c

# Object files
OBJ_SRV := $(patsubst %.c,$(BUILD_DIR)/%.o,$(SRC_SRV))
OBJ_CLT := $(patsubst %.c,$(BUILD_DIR)/%.o,$(SRC_CLT))
OBJ_GUI := $(patsubst %.c,$(BUILD_DIR)/%.o,$(SRC_GUI))
OBJ_BUFFER := $(BUILD_DIR)/$(SRC_BUFFER:.c=.o)
OBJ_LIST := $(BUILD_DIR)/$(SRC_LIST:.c=.o)
OBJ_TEST := $(BUILD_DIR)/$(SRC_TEST:.c=.o)

# Cible par défaut
all: directories $(BIN_SRV) $(BIN_CLT) $(BIN_GUI)

# Création des répertoires
directories:
	@mkdir -p $(BUILD_DIR)/$(SRC_DIR)
	@mkdir -p $(BUILD_DIR)/$(INC_DIR)/buffer
	@mkdir -p $(BUILD_DIR)/$(INC_DIR)/list
	@mkdir -p $(BIN_DIR)

# Exécutables
$(BIN_SRV): $(OBJ_SRV) $(OBJ_LIST)
	$(CC) $(LDFLAGS) $^ -o $@

$(BIN_CLT): $(OBJ_CLT) $(OBJ_BUFFER)
	$(CC) $(LDFLAGS) $^ -o $@

$(BIN_GUI): $(OBJ_GUI) $(OBJ_BUFFER)
	$(CC) $(LDFLAGS) $^ -o $@ $(GTK_LIBS)

$(BIN_TEST): $(OBJ_TEST) $(OBJ_LIST)
	$(CC) $(LDFLAGS) $^ -o $@

# Compilation standard
$(BUILD_DIR)/$(SRC_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/$(INC_DIR)/buffer/%.o: $(INC_DIR)/buffer/%.c
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/$(INC_DIR)/list/%.o: $(INC_DIR)/list/%.c
	$(CC) $(CFLAGS) -c $< -o $@

# Règle spéciale pour GUI (GTK)
$(BUILD_DIR)/$(SRC_DIR)/freescord_gui.o: $(SRC_DIR)/freescord_gui.c
	$(CC) $(CFLAGS) $(GTK_CFLAGS) -c $< -o $@

# Exécutions
serveur: $(BIN_SRV)
	./$(BIN_SRV)

client: $(BIN_CLT)
	./$(BIN_CLT)

gui: $(BIN_GUI)
	./$(BIN_GUI)

test: $(BIN_TEST)
	valgrind --leak-check=full ./$(BIN_TEST)

# Nettoyage
clean:
	rm -rf $(BUILD_DIR) $(BIN_DIR)

# Dépendances
install-deps:
	sudo apt-get update
	sudo apt-get install -y libgtk-3-dev pkg-config

.PHONY: all clean directories serveur client gui test install-deps
