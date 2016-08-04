OPENSSL = /usr/local/opt/openssl
LIBUV = /usr/local/opt/libuv
PROTOBUF = /usr/local/opt/protobuf-c

USER_INCLUDE_PROTO = "../../../PT_Proto/compiled"

OUTPUT = agent
SOURCES = backend.c cluster.c listener.c dispatch.c main.c 
OBJECTS = backend.o cluster.o listener.o dispatch.o main.o 
CC ?= gcc

CFLAGS = -I$(OPENSSL)/include -I$(LIBUV)/include -I$(PROTOBUF)/include -I$(USER_INCLUDE_PROTO) -I"../../common/"
LINK_FLAGS = -L$(USER_INCLUDE_PROTO) -L$(OPENSSL)/lib -L$(LIBUV)/lib -L$(PROTOBUF)/lib -luv -lcrypto -lprotobuf-c -lptnetwork -lproto


$(OUTPUT) : $(OBJECTS)
	$(CC) $(LINK_FLAGS) -o $(OUTPUT) $(OBJECTS)

$(OBJECTS) : $(SOURCES)
	$(CC) -c $(CFLAGS) $(SOURCES)

clean:
	rm -rf *.o
	rm -rf *.a
	rm -rf $(OUTPUT)
