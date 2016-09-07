#include <common/common.h>
#include <time.h>
#include "qtime.h"

const char *get_time()
{
	static char s_time[100];
	time_t rawtime;
	struct tm *timeinfo;
	time(&rawtime);
	timeinfo = localtime(&rawtime);

	sprintf(s_time,"%04d-%02d-%02d %02d:%02d:%02d",timeinfo->tm_year + 1900,timeinfo->tm_mon + 1,timeinfo->tm_mday, timeinfo->tm_hour, timeinfo->tm_min,
			timeinfo->tm_sec);

	return s_time;
}
