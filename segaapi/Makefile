LD=clang #clang
CC=clang
CFLAGS=-m32 -O0 -g
LDFLAGS=$(CFLAGS)

test: test.o libsegaapi.so force
	$(LD) $(LDFLAGS) -L. -lsegaapi -o test test.o
	LD_LIBRARY_PATH="." ./test in.pcm

libsegaapi.so: segaapi.o
	$(LD) $(LDFLAGS) segaapi.o -lalut -fPIC -shared -o libsegaapi.so

clean:
	rm -f test libsegaapi.so

force:
