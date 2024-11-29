CC = g++
CC_FLAGS = -std=c++23 -s -O3 -lncurses -lcurl -lmenu -lpanel -Wall -Wextra 
#debug flags:
#CC_FLAGS = -ggdb -std=c++23 -lncurses -lcurl -lmenu -lpanel -Wall -Wextra

all: build/bin/bakatui

build/bin/bakatui: build/obj/main.o build/obj/net.o build/obj/helper_funcs.o build/obj/main_menu.o build/obj/grades.o
	$(CC) $(CC_FLAGS) build/obj/main.o build/obj/net.o build/obj/helper_funcs.o build/obj/main_menu.o build/obj/grades.o -o build/bin/bakatui

build/obj/main.o: src/main.cpp
	mkdir -p build/obj
	mkdir -p build/bin
	$(CC) $(CC_FLAGS) -c src/main.cpp -o build/obj/main.o

build/obj/net.o: src/net.cpp
	$(CC) $(CC_FLAGS) -c src/net.cpp -o build/obj/net.o

build/obj/helper_funcs.o: src/helper_funcs.cpp
	$(CC) $(CC_FLAGS) -c src/helper_funcs.cpp -o build/obj/helper_funcs.o

build/obj/main_menu.o: src/main_menu.cpp
	$(CC) $(CC_FLAGS) -c src/main_menu.cpp -o build/obj/main_menu.o

build/obj/grades.o: src/grades/*.cpp
	$(CC) $(CC_FLAGS) -c src/grades/*.cpp -o build/obj/grades.o

clean:
	rm -fr build      