//
//  error.c
//  xcode
//
//  Created by 袁睿 on 16/6/22.
//  Copyright © 2016年 number201724. All rights reserved.
//

#include <stdio.h>
#include "error.h"

error_report_cb fatal_cb = NULL;
error_report_cb error_cb = NULL;
error_report_cb log_cb = NULL;

void FATAL(const char *message, const char *function, const char *file, int line)
{
    printf("FATAL:%s\n",message);
    if(fatal_cb){
        fatal_cb(message,function,file,line);
    }
}

void ERROR(const char *message, const char *function, const char *file, int line)
{
    printf("ERROR:%s\n",message);
    
    if(error_cb){
        error_cb(message,function,file,line);
    }
}

void LOG(const char *message, const char *function, const char *file, int line)
{
    printf("LOG:%s\n",message);
    
    if(log_cb){
        log_cb(message,function,file,line);
    }
}

void TRACE(const char *message, const char *function, const char *file, int line)
{
    printf("TRACE:%s\n",message);
}



void set_fatal_filter(error_report_cb cb)
{
    fatal_cb = cb;
}
void set_error_filter(error_report_cb cb)
{
    error_cb = cb;
}
void set_log_filter(error_report_cb cb)
{
    log_cb = cb;
}