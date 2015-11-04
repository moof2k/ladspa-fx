
CC=g++ -I. -Wall -Werror -fPIC -MMD -g

PLUGINS=plugins/lpf.so plugins/biquad.so plugins/biquad_cascade.so
TESTS=lpf_test biquad_test biquad_cascade_test

%.o: %.cpp
	cppcheck $<
	$(CC) -o $@ -c $<

plugins/%.so: %.o
	ld -o $@ $< -shared

%_test: %_test.o
	g++ -g -o $@ $<

all: $(PLUGINS) $(TESTS)

clean:
	rm -rf *.o
	rm -rf *.d
	rm -rf plugins/*.so

-include $(PLUGINS:plugins/%.so=%.d)
-include $(TESTS:%_test=%_test.d)
