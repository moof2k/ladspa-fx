
all:
	gcc -I. -Wall -Werror -O3 -fPIC -o eq.o -c eq.c
	ld -o plugins/eq.so eq.o -shared

