CFLAGS = -c -I. -O2 -DNDEBUG -fPIC
CC = gcc

ifeq ($(shell uname),Darwin)
	SO_EXT := dylib
else
	SO_EXT := so
endif

LIBNAME = libptproto++

OUTPUT = $(LIBNAME).$(SO_EXT)
OUTPUT_STATIC = $(LIBNAME).a

OBJECTS = *.pb.o
SOURCES = *.cc

$(LIBNAME) : $(OBJECTS)
	mkdir -p ../lib
	$(CC) -lprotobuf-c -shared -o../lib/$(OUTPUT) $(OBJECTS)
	ar cr ../lib/$(OUTPUT_STATIC) $(OBJECTS)
	touch $(LIBNAME)

$(OBJECTS) : $(SOURCES)
	$(CC) $(CFLAGS) $(SOURCES)


clean:
	rm -rf *.o
	rm -rf $(LIBNAME)
	rm -rf ../lib/$(OUTPUT)
	rm -rf ../lib/$(OUTPUT_STATIC)


install:
	install *.pb.h /usr/local/include
	install ../lib/$(OUTPUT) /usr/local/lib
	install ../lib/$(OUTPUT_STATIC) /usr/local/lib

uninstall:
	rm -rf /usr/local/include/*.pb.h
	rm -rf /usr/local/lib/$(OUTPUT)
	rm -rf /usr/local/lib/$(OUTPUT_STATIC)
