OPENSSL = /usr/local/opt/openssl
LIBUV = /usr/local/opt/libuv
INSTALL_DIR = /usr/local

OBJECTS = event.o file.o ini.o queue.o coroutine.o commander.o qtime.o
SOURCES = event.c file.c ini.c queue.c coroutine.c commander.c qtime.c

#CFLAGS = -c --std=c99 I$(OPENSSL)/include -I$(LIBUV)/include -I. -O2 -DNDEBUG
CFLAGS = -c -I. -O2 -I$(PT_Service) -DNDEBUG
CC = gcc

ifeq ($(shell uname),Darwin)
	SO_EXT := dylib
else
	SO_EXT := so
	CFLAGS := -fPIC $(CFLAGS)
endif

OUTPUT_SHARE = out/libframework.$(SO_EXT)
OUTPUT = out/libframework.a


$(OUTPUT) : $(OBJECTS)
	mkdir -p out
	ar cr $(OUTPUT) $(OBJECTS) 

$(OBJECTS) : $(SOURCES)
	$(CC) $(CFLAGS) $(SOURCES)


share : $(OBJECTS)
	mkdir -p out
	#$(CC) -o $(OUTPUT_SHARE) -shared $(OBJECTS) -L$(OPENSSL)/lib -L$(LIBUV)/lib -luv -lcrypto -lptnetwork
	$(CC) -o $(OUTPUT_SHARE) -shared $(OBJECTS) -luv -lcrypto -lptnetwork

clean:
	rm -rf *.o
	rm -rf *.a
	rm -rf *.dylib
	rm -rf *.so
	rm -rf out

#install:
#	install ptnetwork.h $(INSTALL_DIR)/include/
#	install -d ptnetwork $(INSTALL_DIR)/include/ptnetwork/
#	install ptnetwork/* $(INSTALL_DIR)/include/ptnetwork/
#	install out/* $(INSTALL_DIR)/lib/


#uninstall:
#	rm -rf $(INSTALL_DIR)/include/ptnetwork.h
#	rm -rf $(INSTALL_DIR)/include/ptnetwork
#	rm -rf $(INSTALL_DIR)/lib/$(OUTPUT)
#	rm -rf $(INSTALL_DIR)/lib/$(OUTPUT_SHARE)
