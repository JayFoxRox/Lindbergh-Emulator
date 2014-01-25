CC=clang -m32
CFLAGS = -g -O0 -fPIC -m32 -D_GNU_SOURCE -Wall -Werror -Wno-unused-variable -Wno-unused-function
LD = g++ -m32
LDFLAGS = -Wl,-z,defs -rdynamic -static-libstdc++ -static-libgcc -lc -ldl -lGL -lglut -lX11 -lm -lpthread -shared -nostdlib #-rdynamic # -Wl,--no-undefined

OBJS=io.o \
     exec.o \
     network.o \
     glx.o \
     glut.o \
     nvidiagl.o \
     main.o

all: lindbergh.so

lindbergh.so: $(OBJS)
	$(LD) $(OBJS) $(LDFLAGS) $(CFLAGS) -o lindbergh.so

clean:
	rm -f lindbergh.so *.o

