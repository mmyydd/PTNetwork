#include "../db_intr/db_intr.hpp"
#include "db_ext.hpp"



db_ext::db_ext(std::string host, uint16_t port, bool is_pipe) :
	db_intr(host,port,is_pipe)
{

}



void db_ext::on_connect_completed()
{

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
