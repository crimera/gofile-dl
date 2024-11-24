build:
	clang -lcurl -lcjson main.c gofile.c -g -o main

gdb: build
	gdb --args ./main

val: build
	valgrind --leak-check=full -s ./main

test: build
	valgrind --leak-check=full -s ./main "https://gofile.io/d/wm0FjD" -d test
