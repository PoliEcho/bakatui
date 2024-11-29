# Compiler and flags
CPPC = g++
CPPC_FLAGS = -std=c++23 -s -O3 -lncurses -lcurl -lmenu -lpanel -Wall -Wextra 
# Debug flags:
# CPPC_FLAGS = -ggdb -std=c++23 -lncurses -lcurl -lmenu -lpanel -Wall -Wextra


SRC_PATH := src
OBJ_PATH := build/obj
BIN_PATH := build/bin


SRC_FILES := $(shell find $(SRC_PATH) -name '*.cpp')
# Generate corresponding object file paths by replacing src/ with build/obj/
OBJ_FILES := $(patsubst $(SRC_PATH)/%.cpp,$(OBJ_PATH)/%.o,$(SRC_FILES))


all: make-build-dir $(BIN_PATH)/bakatui


make-build-dir:
	mkdir -p $(OBJ_PATH)
	mkdir -p $(OBJ_PATH)/marks
	mkdir -p $(BIN_PATH)


$(BIN_PATH)/bakatui: $(OBJ_FILES)
	$(CPPC) $(CPPC_FLAGS) $^ -o $@


$(OBJ_PATH)/%.o: $(SRC_PATH)/%.cpp
	$(CPPC) $(CPPC_FLAGS) -c $< -o $@


clean:
	rm -fr build

.PHONY: all clean make-build-dir
