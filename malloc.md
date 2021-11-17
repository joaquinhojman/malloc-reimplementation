//si x no esta alineado a 4, lo alinea al multiplo de 4 mayor mas cercano
//si x esta alineado a 4 no se producen cambios
#define align4(x) (((((x)-1)>>2)<<2)+4)


//mmap permite mapear una región de los contenidos de un archivo a memoria, 
//y acceder a los mismos directamente como si fuera un array de bytes.

//cuando alloquemos memoria para un bloque, ademas del size pedido hay que allocar 
//sizeof(struct s_bloc)

//hay que guardar la BASE del heap. Malloc revisa si BASE = NULL, de ser asi es la 
//primera vez que lo usamos (find_block?)

//funcion que extienda el heap (extend_heap)

//funcion que corte los bloques (split_block)

/*
MALLOC
si algo falla devuelve NULL

def malloc(size):
    align(size)
    if base != null:
        find_block: busca un trozo de memoria lo suficientemente grande
        si lo encontro:
            split_block: trata de splitear el bloque (tam_bloque > size + meta data)
            marca el pedazo como usado (b->free = 0)
        si no lo encontro:
            extend_heap
    else:
        extend_heap (el heap estara vacio)
*/

/*
FREE

un problema que podemos tener es la extra fragmentacion debido a llamar a malloc
muchas veces. Lo podriamos solucionar combinando los chunks frees con sus vecinos
si estos tambien han sido usados y liberados.

Se deberia validar que el puntero recibido corresponda a un elemento que realmente
fue allocado por malloc. Una primera validacion seria ver que este dentro del heap.

def free(p):
    if p is valid:
        obtenemos la direccion del bloqe
        lo marcamos como libre
        si el bloque previo existe y esta free, fusionamos
        tratamos tambien de fusionar el siguiente bloque
        si estamos en el ultimo bloque del heap, liberamos memoria
        si no hay mas bloques, seteamos base = null
*/
