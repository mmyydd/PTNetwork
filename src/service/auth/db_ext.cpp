#include "db_ext.hpp"


db_ext::db_ext(std::string host, uint16_t port, bool is_pipe) :
	db_intr(host,port,is_pipe)
{

}


db_ext::~db_ext()
{

}

