#include <stddef.h>

typedef struct s_block t_block;
struct s_block {
	t_block *prev;
    t_block *next;
	size_t size;
    int free;
    void* data;
};

void* mymalloc(size_t size);
void myfree(void* ptr);
