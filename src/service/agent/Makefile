OPENSSL = /usr/local/opt/openssl
LIBUV = /usr/local/opt/libuv
PROTOBUF = /usr/local/opt/protobuf-c

OUTPUT = agent
SOURCES = main.c agent.c 
OBJECTS = main.o agent.o
CC ?= gcc

CFLAGS = -I$(OPENSSL)/include -I$(LIBUV)/include -I$(PROTOBUF)/include

LINK_FLAGS = -L$(OPENSSL)/lib -L$(LIBUV)/lib -L$(PROTOBUF)/lib -luv -lcrypto -lprotobuf-c -lptnetwork -lptframework -lptproto


$(OUTPUT) : $(OBJECTS)
	$(CC) $(LINK_FLAGS) -o $(OUTPUT) $(OBJECTS)

$(OBJECTS) : $(SOURCES)
	$(CC) -c $(CFLAGS) $(SOURCES)

clean:
	rm -rf *.o
	rm -rf *.a
	rm -rf $(OUTPUT)
