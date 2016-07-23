//
//  error.h
//  xcode
//
//  Created by 袁睿 on 16/6/22.
//  Copyright © 2016年 number201724. All rights reserved.
//

#ifndef _PT_ERROR_INCLUED_H_
#define _PT_ERROR_INCLUED_H_

enum error_level_enum
{
    ERROR_LEVEL_LOG,
    ERROR_LEVEL_TRACE,
    ERROR_LEVEL_WARNING,
    ERROR_LEVEL_ERROR,
    ERROR_LEVEL_FATAL,



    ERROR_LEVEL_TOTAL
};


void FATAL(const char *message, const char *function, const char *file, int line);
void ERROR(const char *message, const char *function, const char *file, int line);
void LOG(const char *message, const char *function, const char *file, int line);
void TRACE(const char *message, const char *function, const char *file, int line);
void WARNING(const char *message, const char *function, const char *file, int line);

#ifdef NDEBUG
#define DBGPRINT(Message) 
#else
#define DBGPRINT(Message) TRACE(Message,__FUNCTION__,__FILE__,__LINE__)
#endif


typedef void (*error_report_cb)(const char *message, const char *function, const char *file, int line);

void set_error_report(enum error_level_enum level, error_report_cb cb);


#define FATAL_MEMORY_ERROR() { \
        ERROR("Allocate Memory Failed", __FUNCTION__, __FILE__, __LINE__); \
    }

#endif /* error_h */
