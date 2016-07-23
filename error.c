#include <ptnetwork/common.h>
#include <ptnetwork/error.h>

error_report_cb error_callbacks[ERROR_LEVEL_TOTAL] = {NULL};

void FATAL(const char *message, const char *function, const char *file, int line)
{
    if(error_callbacks[ERROR_LEVEL_FATAL])
    {
        error_callbacks[ERROR_LEVEL_FATAL](message,function,file,line);
    }
}

void ERROR(const char *message, const char *function, const char *file, int line)
{
    if(error_callbacks[ERROR_LEVEL_ERROR])
    {
        error_callbacks[ERROR_LEVEL_ERROR](message,function,file,line);
    }
}

void LOG(const char *message, const char *function, const char *file, int line)
{
    if(error_callbacks[ERROR_LEVEL_LOG])
    {
        error_callbacks[ERROR_LEVEL_LOG](message,function,file,line);
    }
}

void TRACE(const char *message, const char *function, const char *file, int line)
{
    if(error_callbacks[ERROR_LEVEL_TRACE])
    {
        error_callbacks[ERROR_LEVEL_TRACE](message,function,file,line);
    }
}

void WARNING(const char *message, const char *function, const char *file, int line)
{
    if(error_callbacks[ERROR_LEVEL_WARNING])
    {
        error_callbacks[ERROR_LEVEL_WARNING](message,function,file,line);
    }
}


void set_error_report(enum error_level_enum level, error_report_cb cb)
{
    assert(level >= ERROR_LEVEL_LOG && level <= ERROR_LEVEL_FATAL);

    error_callbacks[level] = cb;
}
