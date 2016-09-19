#include "db_ext.hpp"
db_ext::db_ext(bool pipe, std::string host, uint16_t port):
	db_intr(pipe, host, port)
{

}


db_ext::~db_ext()
{

}


void db_ext::on_connected()
{
	fprintf(stderr, "connected database service\n");
}

void db_ext::on_disconnected()
{
	fprintf(stderr, "\n");
}

void db_ext::on_connect_failed()
{
	fprintf(stderr, "connect failed\n");
}
