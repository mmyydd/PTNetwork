OPENSSL = /usr/local/opt/openssl
LIBUV = /usr/local/opt/libuv
INSTALL_DIR = /usr/local

OBJECTS = buffer.o buffer_reader.o packet.o table.o crc32.o mymemory.o server.o client.o error.o
SOURCES = buffer.c buffer_reader.c packet.c table.c crc32.c mymemory.c server.c client.c error.c

CFLAGS = -c -I$(OPENSSL)/include -I$(LIBUV)/include -I.
CC = gcc

ifeq ($(shell uname),Darwin)
	SO_EXT := dylib
else
	SO_EXT := so
	CFLAGS := -fPIC $(CFLAGS)
endif


OUTPUT_SHARE = out/libptnetwork.$(SO_EXT)
OUTPUT = out/libptnetwork.a


$(OUTPUT) : $(OBJECTS)
	mkdir -p out
	ar cr $(OUTPUT) $(OBJECTS) 

$(OBJECTS) : $(SOURCES)
	$(CC) $(CFLAGS) $(SOURCES)


share : $(OBJECTS)
	mkdir -p out
	$(CC) -o $(OUTPUT_SHARE) -shared $(OBJECTS) -L$(OPENSSL)/lib -L$(LIBUV)/lib -luv -lcrypto

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
	install out/* $(INSTALL_DIR)/lib/


uninstall:
	rm -rf $(INSTALL_DIR)/include/ptnetwork.h
	rm -rf $(INSTALL_DIR)/include/ptnetwork
	rm -rf $(INSTALL_DIR)/lib/$(OUTPUT)
	rm -rf $(INSTALL_DIR)/lib/$(OUTPUT_SHARE)
