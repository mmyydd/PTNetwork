#include <ptframework/common/common.h>
#include <fstream>
#include "auth_service.hpp"
#include "db_ext.hpp"

using namespace std;
uv_signal_t sigterm_handle;
uv_signal_t sigint_handle;

db_ext *db;

void sig_term()
{

}
int main(int argc, char *argv[])
{
	uv_signal_init(uv_default_loop(), &sigterm_handle);
	uv_signal_init(uv_default_loop(), &sigint_handle);

	db = new db_ext(true, "/var/tmp/private-database.sock", 0);
	db->begin_connect();

	auth_service service;

	service.start(true, "/var/tmp/pt_auth.sock",0);
	uv_run(uv_default_loop(), UV_RUN_DEFAULT);

	return 0;
}
