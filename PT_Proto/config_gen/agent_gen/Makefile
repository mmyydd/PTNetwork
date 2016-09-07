

CC = g++
SOURCES = main.cpp ../../compiled/agent_servers.pb.cc
OBJECTS = main.o agent_servers.pb.o
PROTOBUF_COMPILED_DIR = ../../compiled
TARGET = config_gen

$(TARGET) : $(OBJECTS)
	$(CC) -o $(TARGET) $(OBJECTS) -ljsoncpp -lprotobuf

$(OBJECTS) : $(SOURCES)
	$(CC) -c -I$(PROTOBUF_COMPILED_DIR) $(SOURCES)

#main.o : main.cpp
#	$(CC) -c -I$(PROTOBUF_COMPILED_DIR) main.cpp
#
#network_config.pb.o : $(PROTOBUF_COMPILED_DIR)/network_config.pb.cc
#	$(CC) -c -I$(PROTOBUF_COMPILED_DIR) $(PROTOBUF_COMPILED_DIR)/network_config.pb.cc


