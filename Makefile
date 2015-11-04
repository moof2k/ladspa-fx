
CC=g++ -I. -Wall -Werror -O3 -fPIC -MMD

PLUGINS=plugins/lpf.so plugins/biquad.so plugins/biquad_cascade.so

%.o: %.cpp
	cppcheck $<
	$(CC) -o $@ -c $<

plugins/%.so: %.o
	ld -o $@ $< -shared

all: $(PLUGINS)

clean:
	rm -rf *.o
	rm -rf *.d
	rm -rf plugins/*.so

-include $(PLUGINS:plugins/%.so=%.d)
