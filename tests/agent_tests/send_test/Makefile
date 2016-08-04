OPENSSL = /usr/local/opt/openssl
LIBUV = /usr/local/opt/libuv
PROTOBUF = /usr/local/opt/protobuf-c

OUTPUT = agent
SOURCES = main.c 
OBJECTS = main.o 
CC ?= gcc

CFLAGS = -I$(OPENSSL)/include -I$(LIBUV)/include -I$(PROTOBUF)/include -I"../../common/"
LINK_FLAGS = -L$(OPENSSL)/lib -L$(LIBUV)/lib -L$(PROTOBUF)/lib -luv -lcrypto -lptnetwork 

$(OUTPUT) : $(OBJECTS)
	$(CC) $(LINK_FLAGS) -o $(OUTPUT) $(OBJECTS)

$(OBJECTS) : $(SOURCES)
	$(CC) -c $(CFLAGS) $(SOURCES)

