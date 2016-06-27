//
//  error.h
//  xcode
//
//  Created by 袁睿 on 16/6/22.
//  Copyright © 2016年 number201724. All rights reserved.
//

#ifndef error_h
#define error_h

void FATAL(const char *message, const char *function, const char *file, int line);
void ERROR(const char *message, const char *function, const char *file, int line);
void LOG(const char *message, const char *function, const char *file, int line);

#endif /* error_h */
