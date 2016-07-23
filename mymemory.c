#include <ptnetwork/common.h>
#include <ptnetwork/mymemory.h>
#include <ptnetwork/error.h>

void *emalloc(size_t size, const char *function, const char *file, int line)
{
    void* ptr = malloc(size);
    
    if( ptr == NULL )
    {
        FATAL("allocate memory failed", function,file,line);
    }
    
    return ptr;
}


void efree(void *ptr, const char *function, const char *file, int line)
{
    free(ptr);
}

void* erealloc(void* ptr, size_t n, const char *function, const char *file, int line)
{
    void *nptr = realloc(ptr, n);
    
    if( nptr == NULL )
    {
        FATAL("allocate memory failed", function,file,line);
    }
    
    return nptr;
}


void* ecalloc(size_t count, size_t size, const char *function, const char *file, int line)
{
    void * ptr = calloc(count, size);
    
    if( ptr == NULL )
    {
        FATAL("allocate memory failed", function,file,line);
    }
    
    return ptr;
}
