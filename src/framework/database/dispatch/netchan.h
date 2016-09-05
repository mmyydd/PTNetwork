#ifndef _NETCHAN_INCLUDED_H_
#define _NETCHAN_INCLUDED_H_

#include <db_query.pb-c.h>
//初始化程序结构
void netchan_pre_operation();

//收到SIGTERM后break掉
void netchan_break_operation();

//释放一些资源
void netchan_post_operation();

#endif
