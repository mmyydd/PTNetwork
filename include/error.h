//
//  error.h
//  xcode
//
//  Created by 袁睿 on 16/6/22.
//  Copyright © 2016年 number201724. All rights reserved.
//

#ifndef _PT_ERROR_INCLUED_H_
#define _PT_ERROR_INCLUED_H_

void FATAL(const char *message, const char *function, const char *file, int line);
void ERROR(const char *message, const char *function, const char *file, int line);
void LOG(const char *message, const char *function, const char *file, int line);
void TRACE(const char *message, const char *function, const char *file, int line);

#define DBGPRINT(Message) TRACE(Message,__FUNCTION__,__FILE__,__LINE__)

typedef void (*error_report_cb)(const char *message, const char *function, const char *file, int line);

void set_fatal_filter(error_report_cb cb);
void set_error_filter(error_report_cb cb);
void set_log_filter(error_report_cb cb);

#endif /* error_h */
