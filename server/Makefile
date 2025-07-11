CC = gcc

SRC_DIR = src
BUILD_DIR = build
LIB_DIR = lib

SRC = $(wildcard $(SRC_DIR)/*.c)
OUT = $(BUILD_DIR)/server

LIB_SUBDIRS := $(shell find $(LIB_DIR) -mindepth 1 -maxdepth 1 -type d)
LIB_BUILD_DIRS := $(patsubst %, %/build, $(LIB_SUBDIRS))

ifeq ($(DEPS_ONLY),true)
    ALL_DEPS := build_libs
else
    ALL_DEPS := build_libs $(OUT)
endif

all: $(ALL_DEPS)

$(OUT): $(SRC)
	mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) $^ -o $@

build_libs:
	@for dir in $(LIB_SUBDIRS); do \
		mkdir -p $$dir/build; \
		cmake -S $$dir -B $$dir/build; \
		$(MAKE) -C $$dir/build; \
	done

clean:
	rm -rf $(BUILD_DIR)
	for dir in $(LIB_SUBDIRS); do rm -rf $$dir/build; done

.PHONY: all clean build_libs
