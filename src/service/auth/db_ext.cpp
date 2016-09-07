#include "db_ext.hpp"
#include "service.hpp"


db_ext::db_ext(std::string host, uint16_t port, bool is_pipe) :
	db_intr(host,port,is_pipe)
{

}


db_ext::~db_ext()
{

}


void db_ext::on_connect_completed()
{
	fprintf(stderr, "database connect successful.\n");
}

void db_ext::on_connect_failed()
{

}

void db_ext::on_lost_connection()
{

}

void db_ext::on_error(std::string errtext)
{

}
