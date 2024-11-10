CC = g++
CC_FLAGS = -std=c++23 -s -O3 -lncurses -lcurl -Wall -Wextra 
#debug flags:
#CC_FLAGS = -ggdb -std=c++23 -lncurses -lcurl -Wall -Wextra

all: build/bin/bakatui

build/bin/bakatui: build/obj/main.o build/obj/net.o build/obj/helper_funcs.o
	$(CC) $(CC_FLAGS) build/obj/main.o build/obj/net.o build/obj/helper_funcs.o -o build/bin/bakatui

build/obj/main.o: src/main.cpp
	mkdir -p build/obj
	mkdir -p build/bin
	$(CC) $(CC_FLAGS) -c src/main.cpp -o build/obj/main.o

build/obj/net.o: src/net.cpp
	$(CC) $(CC_FLAGS) -c src/net.cpp -o build/obj/net.o

build/obj/helper_funcs.o: src/helper_funcs.cpp
	$(CC) $(CC_FLAGS) -c src/helper_funcs.cpp -o build/obj/helper_funcs.o 

clean:
	rm -fr build      