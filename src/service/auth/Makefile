INSTALL_DIR = /usr/local

OBJECTS = main.o db_ext.o base64.o auth_service.o
SOURCES = main.cpp db_ext.cpp base64.cpp auth_service.cpp
	

CFLAGS = -c -g -DDEBUG -std=c++11 

LIB_DIR = -L/usr/lib64/mysql
LIBS = -luv -lcrypto -lptnetwork -lprotobuf -lmysqlclient -ljsoncpp -lptframework -lptbase


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

