
CC = emcc

SRC_DIR = src
BUILD_DIR = build

SRC = $(wildcard $(SRC_DIR)/*.c)
OUT_JS = $(BUILD_DIR)/main.js

CFLAGS = -O2
EMCC_FLAGS = -s WASM=1

all: $(OUT_JS)

$(OUT_JS): $(SRC)
	mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) $(EMCC_FLAGS) $^ -o $@

clean:
	rm -rf $(BUILD_DIR)

.PHONY: all clean
