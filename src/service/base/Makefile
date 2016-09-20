OPENSSL = /usr/local/opt/openssl
LIBUV = /usr/local/opt/libuv
INSTALL_DIR = /usr/local

OBJECTS = service.o db_intr.o
SOURCES = service.cpp db_intr.cpp

CFLAGS = -c -std=c++11 -I. -O2 -fPIC -DNDEBUG
CC = g++

ifeq ($(shell uname),Darwin)
	SO_EXT := dylib
else
	SO_EXT := so
endif

OUTPUT_SHARE = libptbase.$(SO_EXT)
OUTPUT = libptbase.a 

all : $(OUTPUT) $(OUTPUT_SHARE)


$(OUTPUT) : $(OBJECTS)
	ar cr $(OUTPUT) $(OBJECTS)


$(OUTPUT_SHARE) : $(OBJECTS)
	$(CC) -shared -luv -lcrypto -lprotobuf -lptproto++ -lptframework -o$(OUTPUT_SHARE) $(OBJECTS)


$(OBJECTS) : $(SOURCES)
	$(CC) $(CFLAGS) $(SOURCES)

install:
	install -d /usr/local/include/ptbase
	install -p -D *.hpp /usr/local/include/ptbase
	install -p -D $(OUTPUT) /usr/local/lib
	install -p -D $(OUTPUT_SHARE) /usr/local/lib

uninstall:
	rm -rf /usr/local/include/ptbase
	rm -rf /usr/local/lib/$(OUTPUT)
	rm -rf /usr/local/lib/$(OUTPUT_SHARE)

clean:
	rm -rf *.o
	rm -rf *.a
	rm -rf *.dylib
	rm -rf *.so
	rm -rf out

