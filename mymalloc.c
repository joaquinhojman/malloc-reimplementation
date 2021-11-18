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

t_block *base = NULL;	// First free region

//extiende el heap en un bloque de tamaño fijo de 16KiB (enunciado)
static t_block* extend_heap(){
    t_block* block = mmap(NULL, TAM_BLOQUE,PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (block == MAP_FAILED) {
        printf("FALLO MMAP EN EXTEND_HEAP\n");
        return NULL;
    }
    block->prev = NULL;
    block->next = NULL;
    block->size = TAM_BLOQUE;
    block->free = LIBRE;
    block->data = block + 1;

    base = block;
    return block;
}

//corta una region
static t_block* split_block(t_block * old_block, size_t size) {
    t_block* new_block = (t_block *)((char*)old_block->data + size);
    new_block->prev = old_block;
    new_block->next = old_block->next;
    new_block->free = LIBRE;
    new_block->size = old_block->size - size - sizeof(t_block);
    new_block->data = new_block + 1;
    
    old_block->next = new_block;
    old_block->size = size;

    if (new_block->next)
        new_block->next->prev = new_block;

    return old_block;
}

//encuentra una region libre
static t_block* find_block(size_t size){
    t_block* block = base;
    for (int _i = 0; _i < TAM_BLOQUE/TAM_MINIMO; _i++) {
        if (block->size > size && block->free == LIBRE)
            return block;
        if (block->next) block = block->next;
        else break;        
    }
    return NULL;
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
    if (base == NULL) {
        printf("Double Free\n"); //la base ya fuera liberada antes, heap vacio
        return;
    }
    t_block* block = (t_block *) ptr - 1;
    if (block->free == LIBRE) {
        printf("Double Free\n");
        return;
    }

    if (block->next) {
        if (block->next->free == LIBRE) {
            block->size += block->next->size + sizeof(t_block);
            if (block->next->next){
                block->next = block->next->next;
                block->next->prev = block;
            } 
        }
    }

    if (block->prev) {
        if (block->prev->free == LIBRE) {
            block->prev->size += block->size + sizeof(t_block);
            if (block->next){
                block->prev->next = block->next;
                block->next->prev = block->prev;
            } 
            block = block->prev;
        }
    }
    
    block->free = LIBRE;
    if (block->size == TAM_BLOQUE) {
        munmap(block,TAM_BLOQUE);
        base = NULL;
    }
}
