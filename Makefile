# Compiler and flags
CPPC = g++
CPPC_FLAGS = -std=c++23 -s -O3 -lncursesw -lcurl -lmenuw -lpanel -Wall -Wextra -Wno-write-strings
DEBUG_FLAGS = -ggdb -std=c++23 -lncursesw -lcurl -lmenuw -lpanel -Wall -Wextra -Wno-write-strings
DEBUG_ASANITIZE = -fsanitize=address -ggdb -fno-omit-frame-pointer -std=c++23 -lncursesw -lcurl -lmenuw -lpanel -Wall -Wextra -Wno-write-strings


SRC_PATH := src
OBJ_PATH := build/obj
BIN_PATH := build/bin


SRC_FILES := $(shell find $(SRC_PATH) -name '*.cpp')
OBJ_FILES := $(patsubst $(SRC_PATH)/%.cpp,$(OBJ_PATH)/%.o,$(SRC_FILES))


all: make-build-dir $(BIN_PATH)/bakatui


debug: CPPC_FLAGS = $(DEBUG_FLAGS)
debug: make-build-dir $(BIN_PATH)/bakatui

asan: CPPC_FLAGS = $(DEBUG_ASANITIZE)
asan: make-build-dir $(BIN_PATH)/bakatui


make-build-dir:
	mkdir -p $(OBJ_PATH)
	mkdir -p $(BIN_PATH)


$(BIN_PATH)/bakatui: $(OBJ_FILES)
	$(CPPC) $(CPPC_FLAGS) $^ -o $@


$(OBJ_PATH)/%.o: $(SRC_PATH)/%.cpp $(SRC_PATH)/%.h
	$(CPPC) $(CPPC_FLAGS) -c $< -o $@


install:
	@install -vpm 755 -o root -g root $(BIN_PATH)/bakatui $(DESTDIR)/usr/bin/

clean:
	rm -fr build

.PHONY: all clean install debug asan
