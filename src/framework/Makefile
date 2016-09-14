OPENSSL = /usr/local/opt/openssl
LIBUV = /usr/local/opt/libuv
INSTALL_DIR = /usr/local

OBJECTS = event.o file.o ini.o queue.o commander.o qtime.o
SOURCES = event.c file.c ini.c queue.c commander.c qtime.c

CFLAGS = -c -I. -O2 -I$(PT_Service) -DNDEBUG
CC = gcc

ifeq ($(shell uname),Darwin)
	SO_EXT := dylib
else
	SO_EXT := so
	CFLAGS := -fPIC $(CFLAGS)
endif

OUTPUT_SHARE = libptframework.$(SO_EXT)
OUTPUT = libptframework.a


$(OUTPUT) : $(OBJECTS)
	$(CC) -o $(OUTPUT_SHARE) -shared $(OBJECTS) -luv -lcrypto -lptnetwork
	ar cr $(OUTPUT) $(OBJECTS) 

$(OBJECTS) : $(SOURCES)
	$(CC) $(CFLAGS) $(SOURCES)

clean:
	rm -rf *.o
	rm -rf *.a
	rm -rf *.dylib
	rm -rf *.so
	rm -rf out

install:
	install -d /usr/local/include/ptframework
	install -d /usr/local/include/ptframework/common
	install -p -D *.h /usr/local/include/ptframework/
	install -p -D common/* /usr/local/include/ptframework/common/
	install $(OUTPUT) /usr/local/lib
	install $(OUTPUT_SHARE) /usr/local/lib


uninstall:
	rm -rf /usr/local/lib/$(OUTPUT)
	rm -rf /usr/local/lib/$(OUTPUT_SHARE)
	rm -rf /usr/local/include/ptframework

