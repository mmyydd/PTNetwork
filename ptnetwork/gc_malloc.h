#ifndef _GCMALLOC_INCLUDED_H_
#define _GCMALLOC_INCLUDED_H_
struct st_gcmalloc_frame;

//压入一个释放帧
void gcmalloc_push_frame(struct st_gcmalloc_frame **your_frame);

//弹出并释放一个帧
void gcmalloc_pop_frame(struct st_gcmalloc_frame **your_frame);

//在当前帧中申请一块大小为n的内存
void* gcmalloc_alloc(struct st_gcmalloc_frame **your_frame, size_t n);
char* gcmalloc_strdup(struct st_gcmalloc_frame **your_frame, char *s);


#define gcmalloc(_SIZE_) gcmalloc_alloc(NULL,_SIZE_)
#define gcmalloc_push() gcmalloc_push_frame(NULL)
#define gcmalloc_pop() gcmalloc_pop_frame(NULL)

#endif
