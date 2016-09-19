#ifndef _DB_EXT_INCLUDED_H_
#define _DB_EXT_INCLUDED_H_

#include <ptbase/db_intr.hpp>

class db_ext : public db_intr
{
public:
	db_ext(bool pipe, std::string host, uint16_t port);
	virtual ~db_ext();

	virtual void on_connect_failed();
	virtual void on_connected();
	virtual void on_disconnected();
};

#endif
