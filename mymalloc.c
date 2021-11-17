#include "mymalloc.h"
#include <sys/mman.h>
#include <stdio.h>

#define OCUPADO 0
#define LIBRE 1 
#define TAM_BLOQUE 16384 //16 KiB
#define TAM_MAXIMO 33554432 //32 MiB 
#define TAM_MINIMO 256
//si x no esta alineado a 4, lo alinea al multiplo de 4 mayor mas cercano
#define ALIGN4(x) (((((x)-1)>>2)<<2)+4)

t_block *base = NULL;	// First free block

//extiende el heap en un bloque de tamaño fijo de 16KiB (enunciado)
static t_block* extend_heap(){
    t_block* block = mmap(NULL, TAM_BLOQUE,PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (block == MAP_FAILED) {
        printf("FALLO MMAP EN EXTEND_HEAP\n");
        return NULL;
    }
    block->size = TAM_BLOQUE;
    block->free = LIBRE;
    block->data = block + sizeof(t_block);

    base = block;
    return block;
}

//corta un bloque
static t_block* split_block(t_block * old_block, size_t size) {
    t_block* new_block = (t_block *)((char*)old_block->data + size);
    new_block->prev = old_block;
    new_block->next = old_block->next;
    new_block->free = LIBRE;
    new_block->size = old_block->size - size - sizeof(t_block);
    new_block->data = new_block + sizeof(t_block);
    
    old_block->next = new_block;
    old_block->size = size;

    if (new_block->next)
        new_block->next->prev = new_block;

    return old_block;
}

//encuentra un bloque libre
static t_block* find_block(size_t size){
    t_block* block = base;
    while (!(block->size > size && block->free == LIBRE)){
        if (block->next) block = block->next;
        else return NULL;        
    }
    return block;
}

void* mymalloc(size_t size) {
    if (!size)
        return NULL;
    size = ALIGN4(size);

    if (size > TAM_MAXIMO || size > TAM_BLOQUE || size < 0) 
        return NULL;
    if (size < TAM_MINIMO) 
        size = TAM_MINIMO;

    t_block *block;
    if (base == NULL) 
        block = extend_heap();
    else 
        block = find_block(size); 

    if (block == NULL) 
        return NULL;
  
    if (block->size > size + sizeof(t_block))
        block = split_block(block,size);

    block->free = OCUPADO;
    return block->data;
}

void myfree(void *ptr){
    if (!ptr) {
        printf("Invalid Free\n");
        return;
    }
    t_block* block = (t_block *) ptr - sizeof(t_block);
    if (block->free == LIBRE) {
        printf("Double Free\n");
        return;
    }

    if (block->next) {
        if (block->next->free == LIBRE) {
            block->size += block->next->size + sizeof(t_block);
            if (block->next->next) block->next = block->next->next;
        }
    }

    if (block->prev) {
        if (block->prev->free == LIBRE) {
            block->prev->size += block->size + sizeof(t_block);
            if (block->next) block->prev->next = block->next;
            block = block->prev;
        }
    }
    
    block->free = LIBRE;
    if (block->size == TAM_BLOQUE) {
        munmap(block,TAM_BLOQUE);
        base = NULL;
    }
}
