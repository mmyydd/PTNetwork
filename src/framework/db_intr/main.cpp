#include "db_intr.hpp"
int query_count = 0;
int prev_query_count = 0;

using namespace std;
uv_timer_t timer;
class myinterface : public db_intr
{
public:
	myinterface(std::string host, uint16_t port, bool is_pipe):
		db_intr(host,port,is_pipe)
	{

	};

	virtual void on_connect_completed()
	{
		printf("connected\n");
		restore_queue_query();
	}

	virtual void on_connect_failed()
	{
		printf("connect failed\n");
	}
	virtual void on_lost_connection()
	{
		printf("on lost connection\n");
	}
};

myinterface myintr("/var/tmp/public-database.sock",0, true);
void on_query_cb(db_intr_handle *pQuery);
void go_query()
{
	db_intr_params kvc;
	myintr.begin_query("auth", "SELECT * FROM users",kvc,on_query_cb);
}

void uv_print(uv_timer_t *timer)
{
	printf("total:%d  qps:%d/s\n",query_count,  query_count - prev_query_count);

	prev_query_count = query_count;
}
void on_query_cb(db_intr_handle *pQuery)
{
	query_count++;

	try{
		db_intr_record_set record(*pQuery);

		int row = 0;


		std::cout << "record count:" << record.get_record_count() << std::endl;
		if(record.move_first())
		{
			do
			{
				int count = record.get_fields_count();

				for(int i =0; i < count; i++)
				{
					std::string res;
					record.get_field_value(i,res);
					std::cout << "res:" << res << std::endl;
				}

				row++;
			}while(record.move_next());
		}
	}
	catch(db_intr_handle_exception &except)
	{
		std::cout << except.what() << std::endl;
	}

	go_query();
}

int main(int argc, char *argv[])
{
	db_intr_params kvc;
	myintr.begin_connect();

	uv_timer_init(uv_default_loop(), &timer);
	uv_timer_start(&timer, uv_print, 1000,1000);
	myintr.begin_query("auth", "SELECT * FROM users",kvc,on_query_cb);

	uv_run(uv_default_loop(), UV_RUN_DEFAULT);
	return 0;
}
