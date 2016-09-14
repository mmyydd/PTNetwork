OPENSSL = /usr/local/opt/openssl
LIBUV = /usr/local/opt/libuv
INSTALL_DIR = /usr/local

OBJECTS = main.o db_ext.o base64.o
SOURCES = main.cpp db_ext.cpp base64.cpp
	

CFLAGS = -c -g -DDEBUG -std=c++11 -I$(PT_Service) -I$(OPENSSL)/include \
		 -I$(LIBUV)/include -I$(PT_Service)/framework -I$(PT_Proto)/compiled -I$(PT_Service)/service/base

LIB_DIR = -L$(OPENSSL)/lib -L/usr/lib64/mysql -L$(LIBUV)/lib -L/usr/local/opt/jsoncpp/lib -L$(PT_Proto)/compiled/out -L$(PT_Service)/service/base/
LIBS = -luv -lcrypto -lptnetwork -lprotobuf -lmysqlclient -ljsoncpp $(PT_Proto)/compiled/out/libpt_proto.a ../base/libptbase.a


CC = g++
OUTPUT = auth

$(OUTPUT) : $(OBJECTS)
	$(CC) -o $(OUTPUT) -g $(LIB_DIR) $(LIBS) $(OBJECTS)

$(OBJECTS) : $(SOURCES)
	$(CC) $(CFLAGS) $(SOURCES)

clean:
	rm -rf *.o
	rm -rf *.a
	rm -rf *.dylib
	rm -rf *.so
	rm -rf $(OUTPUT)

