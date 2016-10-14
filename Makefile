OPENSSL = /usr/local/opt/openssl
LIBUV = /usr/local/opt/libuv
INSTALL_DIR = /usr/local

SOURCES = buffer.c buffer_reader.c packet.c table.c crc32.c mymemory.c server.c client.c error.c gc_malloc.c
OBJECTS = buffer.o buffer_reader.o packet.o table.o crc32.o mymemory.o server.o client.o error.o gc_malloc.o

CFLAGS = -c -I$(OPENSSL)/include -I$(LIBUV)/include -I. -O2 -DNDEBUG
CC = gcc

ifeq ($(shell uname),Darwin)
	SO_EXT := dylib
else
	SO_EXT := so
	CFLAGS := -fPIC $(CFLAGS)
endif


OUTPUT_SHARE = libptnetwork.$(SO_EXT)
OUTPUT = libptnetwork.a

all: $(OUTPUT) $(OUTPUT_SHARE)


$(OUTPUT) : $(OBJECTS)
	ar cr $(OUTPUT) $(OBJECTS) 


$(OUTPUT_SHARE) : $(OBJECTS)
	$(CC) -o $(OUTPUT_SHARE) -shared $(OBJECTS) -L$(OPENSSL)/lib -L$(LIBUV)/lib -luv -lcrypto


$(OBJECTS) : $(SOURCES)
	$(CC) $(CFLAGS) $(SOURCES)


clean:
	rm -rf *.o
	rm -rf *.a
	rm -rf *.dylib
	rm -rf *.so
	rm -rf out

install:
	install ptnetwork.h $(INSTALL_DIR)/include/
	install -d ptnetwork $(INSTALL_DIR)/include/ptnetwork/
	install ptnetwork/* $(INSTALL_DIR)/include/ptnetwork/
	install $(OUTPUT) $(INSTALL_DIR)/lib/
	install $(OUTPUT_SHARE) $(INSTALL_DIR)/lib/


uninstall:
	rm -rf $(INSTALL_DIR)/include/ptnetwork.h
	rm -rf $(INSTALL_DIR)/include/ptnetwork
	rm -rf $(INSTALL_DIR)/lib/$(OUTPUT)
	rm -rf $(INSTALL_DIR)/lib/$(OUTPUT_SHARE)