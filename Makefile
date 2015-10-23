
all:
	g++ -I. -Wall -Werror -O3 -fPIC -o eq.o -c eq.cpp
	ld -o plugins/eq.so eq.o -shared

