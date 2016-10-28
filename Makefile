INSTALL_DIR = /usr/local

OBJECTS = buffer.o buffer_reader.o packet.o table.o crc32.o mymemory.o server.o error.o gc_malloc.o async_client.o sync_client.o

CFLAGS = -c -I./ptnetwork/ -O2 -DNDEBUG -Wmultichar
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
	$(CC) -o $(OUTPUT_SHARE) -shared -L$(OPENSSL)/lib -L$(LIBUV)/lib -luv -lcrypto $(OBJECTS)

buffer.o : buffer.c
	$(CC) $(CFLAGS) buffer.c

buffer_reader.o : buffer_reader.c
	$(CC) $(CFLAGS) buffer_reader.c

packet.o : packet.c
	$(CC) $(CFLAGS) packet.c

table.o : table.c
	$(CC) $(CFLAGS) table.c

crc32.o : crc32.c
	$(CC) $(CFLAGS) crc32.c

mymemory.o : mymemory.c
	$(CC) $(CFLAGS) mymemory.c

server.o : server.c
	$(CC) $(CFLAGS) server.c

error.o : error.c
	$(CC) $(CFLAGS) error.c

gc_malloc.o : gc_malloc.c
	$(CC) $(CFLAGS) gc_malloc.c

async_client.o : async_client.c
	$(CC) $(CFLAGS) async_client.c

sync_client.o : sync_client.c
	$(CC) $(CFLAGS) sync_client.c

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
