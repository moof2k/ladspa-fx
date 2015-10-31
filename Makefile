
CC=g++ -I. -Wall -Werror -O3 -fPIC

%.o: %.cpp
	$(CC) -o $@ -c $<

plugins/%.so: %.o
	ld -o $@ $< -shared

all: plugins/lpf.so

clean:
	rm -rf *.o
	rm -rf plugins/*.so
