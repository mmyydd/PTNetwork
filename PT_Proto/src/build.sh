#! /bin/bash

protoc-c network_config.proto --c_out=../compiled/
protoc-c agent_servers.proto --c_out=../compiled/
protoc-c database_config.proto --c_out=../compiled/
protoc-c db_query.proto --c_out=../compiled/

protoc network_config.proto --cpp_out=../compiled/
protoc agent_servers.proto --cpp_out=../compiled/
protoc database_config.proto --cpp_out=../compiled/
protoc db_query.proto --cpp_out=../compiled/
