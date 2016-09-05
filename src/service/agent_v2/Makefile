OPENSSL = /usr/local/opt/openssl
LIBUV = /usr/local/opt/libuv
PROTOBUF = /usr/local/opt/protobuf-c

USER_INCLUDE_PROTO = "../../PT_Proto/compiled/"

OUTPUT = agent
SOURCES = main.c ../framework/agent.c ../framework/file.c
OBJECTS = main.o agent.o file.o
CC ?= gcc

CFLAGS = -I"../framework" -I$(OPENSSL)/include -I$(LIBUV)/include -I$(PROTOBUF)/include -I$(USER_INCLUDE_PROTO)

LINK_FLAGS = -L$(USER_INCLUDE_PROTO) -L$(OPENSSL)/lib -L$(LIBUV)/lib -L$(PROTOBUF)/lib -luv -lcrypto -lprotobuf-c -lptnetwork $(USER_INCLUDE_PROTO)out/libpt_proto.a


$(OUTPUT) : $(OBJECTS)
	$(CC) $(LINK_FLAGS) -o $(OUTPUT) $(OBJECTS)

$(OBJECTS) : $(SOURCES)
	$(CC) -c $(CFLAGS) $(SOURCES)

clean:
	rm -rf *.o
	rm -rf *.a
	rm -rf $(OUTPUT)
