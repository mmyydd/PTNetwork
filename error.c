#include <ptnetwork/common.h>
#include <ptnetwork/error.h>
#include <stdarg.h>

error_report_cb error_callbacks[ERROR_LEVEL_TOTAL] = {NULL};

void FATAL(const char *message, const char *function, const char *file, int line)
{
    if(error_callbacks[ERROR_LEVEL_FATAL])
    {
        error_callbacks[ERROR_LEVEL_FATAL](message,function,file,line);
    }
	else
	{
		printf("[FATAL]%s:%s:%d:%s",file,function,line, message);
		abort();
	}
}

void ERROR(const char *message, const char *function, const char *file, int line)
{
    if(error_callbacks[ERROR_LEVEL_ERROR])
    {
        error_callbacks[ERROR_LEVEL_ERROR](message,function,file,line);
    }
	else
	{
		printf("[ERROR]%s:%s:%d:%s",file,function,line, message);
	}

}

void LOG(const char *message, const char *function, const char *file, int line)
{
    if(error_callbacks[ERROR_LEVEL_LOG])
    {
        error_callbacks[ERROR_LEVEL_LOG](message,function,file,line);
    }
	else
	{
		printf("[LOG]%s:%s:%d:%s",file,function,line, message);
	}
}

void TRACE(const char *message, const char *function, const char *file, int line)
{
    if(error_callbacks[ERROR_LEVEL_TRACE])
    {
        error_callbacks[ERROR_LEVEL_TRACE](message,function,file,line);
    }
	else
	{
		printf("[TRACE]%s:%s:%d:%s",file,function,line, message);
	}
}

void WARNING(const char *message, const char *function, const char *file, int line)
{
    if(error_callbacks[ERROR_LEVEL_WARNING])
    {
        error_callbacks[ERROR_LEVEL_WARNING](message,function,file,line);
    }
	else
	{
		printf("[WARNING]%s:%s:%d:%s",file,function,line, message);
	}
}


void set_error_report(enum error_level_enum level, error_report_cb cb)
{
    assert(level >= ERROR_LEVEL_LOG && level <= ERROR_LEVEL_FATAL);

    error_callbacks[level] = cb;
}


void private_WriteLog(int level, const char *function, const char *file, int line, const char *fmt, ...)
{
	static char strbuf[8192];
	va_list args;
	va_start(args, fmt);
	vsnprintf(strbuf, sizeof(strbuf), fmt, args);

	switch(level)
	{
		case ERROR_LEVEL_ERROR:
			ERROR(strbuf, function, file, line);
			break;
		case ERROR_LEVEL_LOG:
			LOG(strbuf, function, file, line);
			break;
		case ERROR_LEVEL_FATAL:
			FATAL(strbuf, function, file, line);
			break;
		case ERROR_LEVEL_TRACE:
			TRACE(strbuf, function, file, line);
			break;
		case ERROR_LEVEL_WARNING:
			WARNING(strbuf, function, file, line);
			break;
		default:
			FATAL(strbuf, function, file, line);
			break;
	}
}


void WriteLog(const char *fmt, ...)
{
	static char strbuf[8192];
	va_list args;
	va_start(args, fmt);
	vsnprintf(strbuf, sizeof(strbuf), fmt, args);

	LOG(strbuf, __FUNCTION__, __FILE__, __LINE__);
}
