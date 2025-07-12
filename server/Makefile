CC = gcc
CXX = g++

SRC_DIR = src
BUILD_DIR = build

C_SRC = $(wildcard $(SRC_DIR)/*.c)
CPP_SRC = $(wildcard $(SRC_DIR)/*.cpp)

C_OBJ = $(C_SRC:%.c=$(BUILD_DIR)/%.o)
CPP_OBJ = $(CPP_SRC:%.cpp=$(BUILD_DIR)/%.o)

OUT = $(BUILD_DIR)/server

CFLAGS = -Wall -O2
CXXFLAGS = -Wall -O2
LDFLAGS = -lwebsockets

all: $(OUT)

$(BUILD_DIR)/%.o: %.c
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/%.o: %.cpp
	mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OUT): $(C_OBJ) $(CPP_OBJ)
	mkdir -p $(BUILD_DIR)
	$(CXX) $^ $(LDFLAGS) -o $@

clean:
	rm -rf $(BUILD_DIR)

.PHONY: all clean
