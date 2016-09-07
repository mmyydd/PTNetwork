OPENSSL = /usr/local/opt/openssl
LIBUV = /usr/local/opt/libuv
INSTALL_DIR = /usr/local
FRAMEWORK =../../framework/out/libframework.a
PT_PROTO = ../../../PT_Proto/compiled/

OBJECTS = main.o db_ext.o db_intr.o db_query.pb.o service.o base64.o
SOURCES = main.cpp db_ext.cpp ../../framework/db_intr/db_intr.cpp ../../../PT_Proto/compiled/db_query.pb.cc service.cpp base64.cpp
	

CFLAGS = -c -g -DDEBUG -std=c++11 -I$(PT_Service) -I$(OPENSSL)/include -I$(LIBUV)/include -I. -I"../../framework/" -I$(PT_PROTO)

CC = g++

OUTPUT = auth

$(OUTPUT) : $(OBJECTS)
	$(CC) -o $(OUTPUT) -g -L$(OPENSSL)/lib -L/usr/lib64/mysql -L$(LIBUV)/lib -L$(PT_PROTO)/out -luv -lcrypto -lptnetwork $(OBJECTS) $(FRAMEWORK) $(PT_PROTO)/out/libpt_proto.a -lprotobuf -lmysqlclient -ljsoncpp

$(OBJECTS) : $(SOURCES)
	$(CC) $(CFLAGS) $(SOURCES)

clean:
	rm -rf *.o
	rm -rf *.a
	rm -rf *.dylib
	rm -rf *.so
	rm -rf $(OUTPUT)

