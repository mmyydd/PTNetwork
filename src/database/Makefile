OPENSSL = /usr/local/opt/openssl
LIBUV = /usr/local/opt/libuv
INSTALL_DIR = /usr/local

OBJECTS = coroutine.o
SOURCES = coroutine.c

CFLAGS = -c -I$(OPENSSL)/include -I$(LIBUV)/include -I.
CC = gcc

ifeq ($(shell uname),Darwin)
	SO_EXT := dylib
else
	SO_EXT := so
	CFLAGS := -fPIC $(CFLAGS)
endif


OUTPUT = ../../bin/coroutine.$(SO_EXT)

$(OUTPUT) : $(OBJECTS)
	$(CC) -o $(OUTPUT) -shared -L$(OPENSSL)/lib -L$(LIBUV)/lib -luv -lcrypto -lptnetwork $(OBJECTS)

$(OBJECTS) : $(SOURCES)
	$(CC) $(CFLAGS) $(SOURCES)

main.o:main.c
	$(CC) $(CFLAGS) main.c

clean:
	rm -rf *.o
	rm -rf *.a
	rm -rf *.dylib
	rm -rf *.so
	rm -rf out

test: main.o $(OBJECTS)
	$(CC) -o test -L$(OPENSSL)/lib -L$(LIBUV)/lib -luv -lcrypto -lptnetwork $(OBJECTS) main.o
