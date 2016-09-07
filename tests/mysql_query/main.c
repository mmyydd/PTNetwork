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

	const char * sql = "select * from users";
	//const char *username= "3";
	if(mysql_real_connect(&conn, "10.211.55.4", "root", "", "PT_Web", 3306,NULL, 0) == NULL){
		printf("Connect to Mysql Database Failed\n");
		exit(1);
	}
	bzero(&bind, sizeof(bind));

	stmt = mysql_stmt_init(&conn);

	if(mysql_stmt_prepare(stmt,sql, (unsigned long)strlen(sql))){
		printf("mysql stmt_prepare failed\n");
		exit(1);
	}

	//bind.buffer_type = MYSQL_TYPE_STRING;
	//bind.buffer = (void*)username;
	//bind.buffer_length = (unsigned long) strlen(username);

	//if(mysql_stmt_bind_param(stmt, &bind)){
//		printf("param bind failed\n");
//		exit(1);
//	}

	MYSQL_RES *result = mysql_stmt_result_metadata(stmt);	
	int num_fields = mysql_stmt_field_count(stmt);
	printf("num  fields:%d\n", num_fields);

	
	unsigned long *result_lens = malloc(sizeof(unsigned long) * num_fields);
	MYSQL_BIND *result_data = malloc(sizeof(MYSQL_BIND) * num_fields);
	bzero(result_data, sizeof(MYSQL_BIND) * num_fields);
	bzero(result_lens, sizeof(unsigned long) * num_fields);
	int i;
	for(i =0 ; i < num_fields; i++)
	{
		result_data[i].buffer = 0;
		result_data[i].buffer_length = 0;
		result_data[i].length = &result_lens[i];


		printf("field:%d  name:%s\n", i, result->fields[i].name);
	}

	if(mysql_stmt_bind_result(stmt, result_data)){
		printf("mysql stmt bind result failed\n");
	}

	if(mysql_stmt_execute(stmt)){
		printf("mysql_stmt_execute failed\n");
		exit(1);
	}

	mysql_stmt_store_result(stmt);
	
	printf("num_rows:%llu\n", mysql_stmt_num_rows(stmt));
	int r;
	void *data;
	while(!(r = mysql_stmt_fetch(stmt)) || r == MYSQL_DATA_TRUNCATED)
	{
		printf("stmt fetcn\n");
		for(i = 0; i < num_fields; i++)
		{
			data = malloc(result_lens[i]);
			result_data[i].buffer = data;
			result_data[i].buffer_length = result_lens[i];
			result_data[i].is_null = malloc(sizeof(my_bool));
			
			mysql_stmt_fetch_column(stmt, &result_data[i], i, 0);

			printf("is_null:%d\n", *result_data[i].is_null);
			printf("%s\n", data);

		}


	}

//	printf("%d\n", r == MYSQL_NO_DATA);
//	printf("%d\n", r == MYSQL_DATA_TRUNCATED);
	
}
