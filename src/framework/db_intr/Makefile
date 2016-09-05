OPENSSL = /usr/local/opt/openssl
LIBUV = /usr/local/opt/libuv
INSTALL_DIR = /usr/local
FRAMEWORK =../framework/out/libframework.a
PT_PROTO = ../../PT_Proto/compiled/

SOURCES = main.cpp db_intr.cpp ../../PT_Proto/compiled/db_query.pb.cc
OBJECTS = main.o db_intr.o db_query.pb.o
	

CFLAGS = -c -g -DDEBUG -std=c++11 -I$(OPENSSL)/include -I$(LIBUV)/include -I. -I"../framework/" -I$(PT_PROTO)
CC = g++

OUTPUT = db_test

$(OUTPUT) : $(OBJECTS)
	$(CC) -o $(OUTPUT) -g -L$(OPENSSL)/lib -L$(LIBUV)/lib -L$(PT_PROTO)/out -luv -lcrypto -lptnetwork $(OBJECTS) $(FRAMEWORK) $(PT_PROTO)/out/libpt_proto.a -lprotobuf \
	-lmysqlclient

$(OBJECTS) : $(SOURCES)
	$(CC) $(CFLAGS) $(SOURCES)

clean:
	rm -rf *.o
	rm -rf *.a
	rm -rf *.dylib
	rm -rf *.so
	rm -rf $(OUTPUT)

