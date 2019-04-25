all: libxdg.so test.bin
xdg.o: xdg.c xdg.h
	$(CC) -c xdg.c -fpic
libxdg.so: xdg.o
	$(CC) -shared -fpic -o libxdg.so xdg.o
test.bin: test.c
	$(CC) -o test.bin test.c -L. -lxdg
clean:
	rm -f xdg.o libxdg.so test.bin
