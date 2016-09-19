#include <stdio.h>
#include <mysql/mysql.h>
#include <stdlib.h>
#include <string.h>

MYSQL conn;

int main(int argc ,char *argv[])
{
	MYSQL_STMT *stmt;
	MYSQL_BIND bind;
	mysql_init(&conn);

	const char * sql = "call p_test(@rank)";
	if(mysql_real_connect(&conn, "10.0.0.4", "root", "123456", "PT_CSGO", 3306,NULL, 0) == NULL){
		printf("Connect to Mysql Database Failed\n");
		exit(1);
	}
	bzero(&bind, sizeof(bind));

	stmt = mysql_stmt_init(&conn);

	if(mysql_stmt_prepare(stmt,sql, (unsigned long)strlen(sql))){
		printf("mysql stmt_prepare failed\n");
		exit(1);
	}
	
	mysql_stmt_execute(stmt);
	int r;
	MYSQL_RES *meta_data;

	do
	{	
		r = mysql_stmt_store_result(stmt);
		meta_data = mysql_stmt_param_metadata(stmt);

		unsigned int field_count = mysql_stmt_field_count(stmt);
	
		printf("field_count:%u\n", field_count);
		printf("row_count:%llu\n", mysql_stmt_num_rows(stmt));
		printf("aff:%llu\n", mysql_stmt_affected_rows(stmt));

		r = mysql_stmt_next_result(stmt);
		printf("next:%d\n", r);

		if(r != 0) break;
	}while(1);

}
