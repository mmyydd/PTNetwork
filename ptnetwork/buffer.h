#ifndef _PT_BUFFER_INCLUED_H_
#define _PT_BUFFER_INCLUED_H_


#define PAGESIZE 0x1000

#define ALIGN_SIZE(n,a) n % a == 0 ? n : ((n / a)+1) * a

/**
 * pt_buffer 流式缓冲区
 */
struct pt_buffer
{
    struct pt_buffer *next;
	
    unsigned char *buff;
	
    uint32_t length;
	
    uint32_t max_length;
    
    uint32_t ref_count;
};

/*
    buffer_allocator的缓冲
 */
struct pt_buffer_allocator
{
    uint32_t buffer_count;
    uint32_t buffer_max_count;
    qboolean enable;
    struct pt_buffer *buff;
};

//获取pt_buffer的借出次数
uint64_t pt_buffer_get_borrow_count();
//获取pt_buffer的还回次数
uint64_t pt_buffer_get_back_count();
//获取pt_buffer的使用量
uint64_t pt_buffer_get_use_count();

//新增引用计数
uint32_t pt_buffer_ref_increment(struct pt_buffer *buff);
uint32_t pt_buffer_ref_decrement(struct pt_buffer *buff);

//申请一个新的pt_buffer 如果buffer_allocator为enable，则使用buffer_allocator申请
//length 可以被设置为0
struct pt_buffer* pt_buffer_new(uint32_t length);
//释放一个pt_buffer 如果已启用buffer_allocator 则使用buffer_allocator释放
void pt_buffer_free(struct pt_buffer *buff);

//为pt_buffer预留多少字节的空间，如果空间不足则执行realloc
void pt_buffer_reserve(struct pt_buffer *buff, uint32_t length);

//将数据追加到pt_buffer的尾部，如果空间不足则会自动申请
void pt_buffer_write(struct pt_buffer *buff, const void *data, uint32_t length);

//将数据从pt_buffer的头部读取数据，并从pt_buffer中删除已经读区的数据
qboolean pt_buffer_read(struct pt_buffer *buff,void *data, uint32_t length, qboolean remove);



//allocator manager
/*
    清空buffer_allocator中的缓存数据
*/
void pt_buffer_clear_allocator();
/*
    是否启用buffer_allocator
 */
void pt_buffer_enable_allocator(qboolean enable);

/*
    设置buffer_allocator的最大缓存数量
 */
void pt_buffer_set_allocator_count(uint32_t count);


//打印buffer数据
void DUMP(struct pt_buffer *buff);
#endif
