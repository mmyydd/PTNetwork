#ifndef _MY_MEMORY_INCLUDED_H_ 
#define _MY_MEMORY_INCLUDED_H_

void *emalloc(size_t size, const char *function, const char *file, int line);
void efree(void *ptr, const char *function, const char *file, int line);
void* erealloc(void* ptr, size_t n, const char *function, const char *file, int line);
void* ecalloc(size_t count, size_t size, const char *function, const char *file, int line);

void enable_memory_debugger();
void dump_leak();

#define XMEM_MALLOC(size) emalloc(size, __FUNCTION__, __FILE__, __LINE__)
#define XMEM_FREE(ptr) efree(ptr, __FUNCTION__, __FILE__, __LINE__)
#define XMEM_REALLOC(ptr, size) erealloc(ptr, size, __FUNCTION__, __FILE__, __LINE__)
#define XMEM_CALLOC(count, size) ecalloc(count, size, __FUNCTION__, __FILE__, __LINE__)


#endif /* ememory_h */
