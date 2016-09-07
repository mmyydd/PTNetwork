#ifndef _DB_POOL_INCLUDED_H_
#define _DB_POOL_INCLUDED_H_


#include <db_query.pb-c.h>
#include <database_config.pb-c.h>
#include <mysql.h>

struct db_config
{
	char *name;
	char *address;
	uint16_t port;
	char *username;
	char *password;
	char *dbname;
};

struct db_conn
{
	struct db_config config;
	MYSQL conn;
};

struct db_pool
{
	int num_of_conn;
	struct db_conn *connections;
};

void db_pool_init(DatabaseConfig *dbConfig);
qboolean db_pool_connect();
struct db_conn* db_pool_getconn(const char *name);
#endif
