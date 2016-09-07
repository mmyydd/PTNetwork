#include <common/common.h>
#include "db_ext.hpp"
#include "service.hpp"

int main(int argc, char *argv[])
{
	g_service.init();
	g_service.startup();
	g_service.run();
	g_service.shutdown();
	return 0;
}
