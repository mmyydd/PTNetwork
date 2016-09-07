

CC = g++
SOURCES = main.cpp
OBJECTS = main.o database_config.pb.o
PROTOBUF_COMPILED_DIR = ../../compiled
TARGET = config_gen

$(TARGET) : $(OBJECTS)
	$(CC) -o $(TARGET) $(OBJECTS) -ljsoncpp -lprotobuf


main.o : main.cpp
	$(CC) -c -I$(PROTOBUF_COMPILED_DIR) main.cpp

database_config.pb.o : $(PROTOBUF_COMPILED_DIR)/database_config.pb.cc
	$(CC) -c -I$(PROTOBUF_COMPILED_DIR) $(PROTOBUF_COMPILED_DIR)/database_config.pb.cc


