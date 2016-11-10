#include "common.h"
#include "table.h"
#include "pt_error.h"


struct pt_table *pt_table_new_ex(int granularity)
{
    struct pt_table *ptable;
    
    
    ptable =(struct pt_table*)XMEM_MALLOC(sizeof(struct pt_table));
    
    bzero(ptable,sizeof(struct pt_table));
    
    ptable->granularity = granularity;
    ptable->size = 0;
    ptable->head = (struct pt_table_node**)XMEM_MALLOC(sizeof(struct pt_table_node*) * ptable->granularity);
    
    bzero(ptable->head, sizeof(struct pt_table_node**) * ptable->granularity);
    
    return ptable;
}


struct pt_table *pt_table_new()
{
	return pt_table_new_ex(TABLE_DEFAULT_GRANULARITY);
   /* struct pt_table *ptable;
    
    ptable =(struct pt_table*)MEM_MALLOC(sizeof(struct pt_table));
    
    bzero(ptable,sizeof(struct pt_table));
    
    ptable->granularity = TABLE_NORMAL_COUNT;
    ptable->size = 0;
    ptable->head = (struct pt_table_node**)MEM_MALLOC(sizeof(struct pt_table_node*) * ptable->granularity);
    
    bzero(ptable->head, sizeof(struct pt_table_node**) * ptable->granularity);
    
    return ptable;
	*/
}


static void pt_table_free_node(struct pt_table_node** pnode)
{
    struct pt_table_node* node;
    struct pt_table_node* tmp;
    node = *pnode;
    
    
    while(node)
    {
        tmp = node->next;
        XMEM_FREE(node);
        node = tmp;
    }
    
    *pnode = NULL;
}

void pt_table_clear(struct pt_table *ptable)
{
    uint32_t i;
    
    for(i = 0; i < ptable->granularity; i++)
    {
        pt_table_free_node(&ptable->head[i]);
        ptable->head[i] = NULL;
    }
    
    ptable->size = 0;
}


void pt_table_free(struct pt_table *ptable)
{
    pt_table_clear(ptable);
    
    XMEM_FREE(ptable->head);
    XMEM_FREE(ptable);
}


static struct pt_table_node* pt_table_node_new()
{
    struct pt_table_node *node = NULL;
    
    node = (struct pt_table_node*)XMEM_MALLOC(sizeof(struct pt_table_node));
    if(node == NULL){
        FATAL_MEMORY_ERROR();
    }
    
    bzero(node,sizeof(struct pt_table_node));
    
    return node;
}

void pt_table_insert(struct pt_table *ptable, uint64_t id, void* ptr)
{
    uint32_t index = id % ptable->granularity;
    struct pt_table_node *current;
    struct pt_table_node *node;
    node = pt_table_node_new();
	node->id = id;
	node->ptr = ptr;
    if(ptable->head[index] == NULL)
    {
		ptable->head[index] = node;
        ptable->size++;
    }
    else
    {
		node->next = ptable->head[index];
		ptable->head[index] = node;
        //current = ptable->head[index];
        //while(current->next != NULL) current = current->next;
        
        //current->next = pt_table_node_new();
        //current->next->id = id;
        //current->next->ptr = ptr;
        ptable->size++;
    }
}

void pt_table_erase(struct pt_table *ptable, uint64_t id)
{
    uint32_t index = id % ptable->granularity;
    struct pt_table_node *current;
    struct pt_table_node *prev;
    current = ptable->head[index];
    prev = NULL;
    
    
    if(current)
    {
        do
        {
            if(current->id == id)
            {
                if(prev)
                {
                    ptable->size--;
                    prev->next = current->next;
                }
                else
                {
                    ptable->size--;
                    ptable->head[index] = current->next;
                }
                
                XMEM_FREE(current);
                break;
            }
            
            prev = current;
            current = current->next;
        }while(current);
    }	
}

void* pt_table_find(struct pt_table *ptable, uint64_t id)
{
    void* ptr = NULL;
    uint32_t index = id % ptable->granularity;
    struct pt_table_node *current;
    
    current = ptable->head[index];
    
    while(current)
    {
        if(current->id == id){
            ptr = current->ptr;
            break;
        }
        
        current = current->next;
    }
    
    return ptr;
}

uint32_t pt_table_size(struct pt_table *ptable)
{
    return ptable->size;
}

void pt_table_enum(struct pt_table *ptable, pt_table_enum_cb cb,void *user_arg)
{
    uint32_t i;
    struct pt_table_node *current, *temp;

	current = NULL;
	temp = NULL;

    for(i = 0; i <ptable->granularity; i++)
    {
        current = ptable->head[i];

        while(current){
			temp = current->next;
            cb(ptable, current->id, current->ptr, user_arg);
            current = temp;
        }
    }
}
