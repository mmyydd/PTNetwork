#include <ptframework/common/common.h>
#include <ptbase/db_intr.hpp>

class db_ext : public db_intr
{
public:
	db_ext(bool pipe, std::string h, uint16_t port): db_intr(pipe, h, port)
	{
	}


	virtual ~db_ext()
	{
	}

	virtual void on_connected()
	{
		fprintf(stderr, "connected\n");
	}

	virtual void on_disconnected()
	{
		fprintf(stderr, "disconnected\n");
	}

	virtual void on_connect_failed()
	{
		fprintf(stderr, "connect failed\n");
	}
};
uv_timer_t timer;

db_ext ext(true, "/var/tmp/public-database.sock",0);
uint32_t complete_count = 0;

void on_query_result(db_intr_handle *handle, void *arg)
{
	fprintf(stderr, "has_error :%d\n",handle->has_error());
	
	db_intr_result_c results = handle->get_results();
	for(uint32_t i = 0; i< results.size(); i++)
	{
		db_intr_fields fields = results[i].get_fields();
		
		fprintf(stderr, "============================\n");
		fprintf(stderr, "result row:%u\n", i);

		for(uint32_t j = 0; j < fields.get_fields_count(); j++)
		{
			std::cout << "\t" << fields.get_field(j).name << std::endl;
		}
	}
	fprintf(stderr, "============================\n");

	for(uint32_t i =0; i< results.size(); i++)
	{
		db_intr_record_set record_set(results[i]);
		fprintf(stderr, "==============================================\n");
		fprintf(stderr, "result row:%u    row_count:%u\n", i, record_set.get_record_count());

		if(record_set.move_first())
		{
			do
			{
				int fields_count = record_set.get_fields_count();

				std::cout << ">>>>>>>>>" << std::endl;
				for(int j = 0; j < fields_count; j++)
				{
					std::string c;
					record_set.get_field_value(j, c);

					std::cout << "value:" << j << "          " <<c << std::endl;
				}

			}while(record_set.move_next());
		}
	}


	fprintf(stderr, "=================================================================\n");
/*	
	
	try
	{
		fprintf(stderr, "result count:%d\n", handle->get_results_count());
		fflush(stderr);
		db_intr_record_set record_set(handle);

		fprintf(stderr, "record_count:%u\n", record_set.get_record_count());
		fflush(stderr);

		fprintf(stderr, "fields print ok\n");
		fflush(stderr);
		
		if(record_set.move_first())
		{
			do
			{
			int fields_count = record_set.get_fields_count();

			std::cout << "==========================================" << std::endl;
			for(int j = 0; j < fields_count; j++)
			{
				std::string c;
				record_set.get_field_value(j, c);

				std::cout << "value:" << j << "          " <<c << std::endl;
			}

			}while(record_set.move_next());
		}
		complete_count ++;
	}
	catch(db_intr_handle_exception &exce)
	{
		fprintf(stderr, "catch:%s\n", exce.what());
	}
	*/
}
void on_timer(uv_timer_t *tm)
{
	db_intr_value_c params;
	params.push_back(db_intr_value(3600));
	ext.begin_query("csgo", "call p_test(@rank, ?)",params, on_query_result,NULL);
}

uv_timer_t timer2;
void print_info(uv_timer_t *timex)
{
	fprintf(stderr, "async count:%u\ncomplete count:%u\n", ext.get_busy_count(), complete_count);
}
int main()
{	
	uv_timer_init(uv_default_loop(), &timer);
	uv_timer_init(uv_default_loop(), &timer2);
	uv_timer_start(&timer, on_timer, 1000,1000);
	uv_timer_start(&timer2, print_info, 1000,1000);

	ext.begin_connect();

	uv_run(uv_default_loop(), UV_RUN_DEFAULT);

	return 0;
}
