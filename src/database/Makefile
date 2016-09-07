OPENSSL = /usr/local/opt/openssl
LIBUV = /usr/local/opt/libuv
INSTALL_DIR = /usr/local
FRAMEWORK =../framework/out/libframework.a
PT_PROTO = ../../PT_Proto/compiled/
MYSQL_HEADER = /usr/include/mysql/
MYSQL_LIB=/usr/lib64/mysql/

PRIVATE_CHANNEL = "\"/var/tmp/private-database.sock\""
PUBLIC_CHANNEL = "\"/var/tmp/public-database.sock\""
PID_FILE = "\"/var/tmp/pt_database.pid\""
DB_CONFIG = "\"database_config.dat\""
NETWORK_CONFIG = "\"network_config.dat\""
LOG_DIR = "\"/var/tmp/pt_service/\""

OBJECTS = main.o daemon.o logic.o listener.o debug.o signal.o netchan.o query.o db_pool.o exec.o

SOURCES = main.c \
		  debug.c \
		  ./daemon/daemon.c \
		  ./daemon/signal.c \
		  ./worker/logic.c \
		  ./dispatch/listener.c \
		  ./dispatch/netchan.c \
		  ./dispatch/query.c \
		  ./worker/db_pool.c \
		  ./worker/exec.c

	

CFLAGS = -c -std=gnu99 -g -I$(PT_Service) -I$(OPENSSL)/include -I$(LIBUV)/include -I$(MYSQL_HEADER) -I. -I"../framework/" -I$(PT_PROTO) -DPRIVATE_CHANNEL=$(PRIVATE_CHANNEL) -DPUBLIC_CHANNEL=$(PUBLIC_CHANNEL) -DPID_FILE=$(PID_FILE) -DDB_CONFIG=$(DB_CONFIG) -DNETWORK_CONFIG=$(NETWORK_CONFIG) -DLOG_DIR=$(LOG_DIR) -DDEBUG

CC = gcc

OUTPUT = database

$(OUTPUT) : $(OBJECTS)
	$(CC) -o $(OUTPUT) -g -L$(OPENSSL)/lib -L$(LIBUV)/lib -L$(MYSQL_LIB) -L$(PT_PROTO)/out -luv -lcrypto -lptnetwork $(OBJECTS) $(FRAMEWORK) $(PT_PROTO)/out/libpt_proto.a -lprotobuf-c \
	-lmysqlclient

$(OBJECTS) : $(SOURCES)
	$(CC) $(CFLAGS) $(SOURCES)

clean:
	rm -rf *.o
	rm -rf *.a
	rm -rf *.dylib
	rm -rf *.so
	rm -rf $(OUTPUT)

