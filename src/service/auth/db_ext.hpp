#ifndef _DB_EXT_INCLUDED_H_
#define _DB_EXT_INCLUDED_H_

#include <db_intr.hpp>

class db_ext : public db_intr
{
public:
	db_ext(std::string host, uint16_t port, bool is_pipe);
	virtual ~db_ext();


	virtual void on_connect_completed(){
	}
	virtual void on_connect_failed(){
	}
	virtual void on_lost_connection(){
	}
};

#endif
