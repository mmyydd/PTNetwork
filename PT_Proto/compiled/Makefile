OPENSSL = /usr/local/opt/openssl
LIBUV = /usr/local/opt/libuv
INSTALL_DIR = /usr/local


OBJECTS = agent_servers.pb-c.o database_config.pb-c.o network_config.pb-c.o db_query.pb-c.o
SOURCES = agent_servers.pb-c.c database_config.pb-c.c network_config.pb-c.c db_query.pb-c.c

CFLAGS = -c -I$(OPENSSL)/include -I$(LIBUV)/include -I. -O2 -DNDEBUG
CC = gcc

ifeq ($(shell uname),Darwin)
	SO_EXT := dylib
else
	SO_EXT := so
	CFLAGS := -fPIC $(CFLAGS)
endif

OUTPUT_SHARE = out/libpt_proto.$(SO_EXT)
OUTPUT = out/libpt_proto.a

$(OUTPUT) : $(OBJECTS)
	mkdir -p out
	ar cr $(OUTPUT) $(OBJECTS) 

$(OBJECTS) : $(SOURCES)
	$(CC) $(CFLAGS) $(SOURCES)


share : $(OBJECTS)
	mkdir -p out
	$(CC) -o $(OUTPUT_SHARE) -shared $(OBJECTS) -L$(OPENSSL)/lib -L$(LIBUV)/lib -luv -lcrypto -lptnetwork -lprotobuf-c

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
