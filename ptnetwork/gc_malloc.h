#ifndef _GCMALLOC_INCLUDED_H_
#define _GCMALLOC_INCLUDED_H_


//压入一个释放帧
void gcmalloc_push_frame();

//弹出并释放一个帧
void gcmalloc_pop_frame();

//在当前帧中申请一块大小为n的内存
void* gcmalloc_alloc(size_t n);

#endif
