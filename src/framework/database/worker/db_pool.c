#include <common.h>
#include "db_pool.h"


struct db_pool dbpool;


void db_conn_init(struct db_conn *conn, DatabaseNode *node)
{
	conn->config.name = strdup(node->name);
	conn->config.address = strdup(node->address);
	conn->config.port = (uint16_t)node->port;
	conn->config.username = strdup(node->username);
	conn->config.password = strdup(node->password);
	conn->config.dbname = strdup(node->dbname);
	

	fprintf(stderr, "initialize mysql connection:%s    address:%s:%d\n",conn->config.name, conn->config.address,
			conn->config.port);

	mysql_init(&conn->conn);
}

qboolean db_conn_connect(struct db_conn *conn)
{
	MYSQL *result;
	if(conn->config.address[0] == '/')
	{
		result = mysql_real_connect(&conn->conn,
				NULL,conn->config.username,
				conn->config.password,conn->config.dbname,
				0,conn->config.address,0);
	}
	else
	{
		result = mysql_real_connect(&conn->conn,conn->config.address,
				conn->config.username,conn->config.password,
				conn->config.dbname,conn->config.port,NULL,0);
	}

	if(result == NULL)
	{
		WriteLog("[LOGIC] DBConnect Failed:%s\n", conn->config.name);
		return false;
	}

	WriteLog("[LOGIC] DBConnect Success:%s\n", conn->config.name);
	return true;
}

void db_pool_init(DatabaseConfig *dbConfig)
{
	size_t i;
	dbpool.num_of_conn = (int)dbConfig->n_databases;
	dbpool.connections = malloc(sizeof(struct db_conn) * dbConfig->n_databases);
	
	fprintf(stderr, "db_pool_init: init %d connection\n", dbpool.num_of_conn);

	for(i = 0; i < dbConfig->n_databases; i++)
	{
		db_conn_init(&dbpool.connections[i], dbConfig->databases[i]);
	}
}


qboolean db_pool_connect()
{
	int i;

	for(i = 0; i < dbpool.num_of_conn;i++)
	{
		if(db_conn_connect(&dbpool.connections[i]) == false)
		{
			return false;
		}
	}

	return true;
}


struct db_conn* db_pool_getconn(const char *name)
{
	int i;

	for(i =0; i < dbpool.num_of_conn;i++)
	{
		if(!strcmp(dbpool.connections[i].config.name, name))
		{
			return &dbpool.connections[i];
		}
	}

	return NULL;
}


