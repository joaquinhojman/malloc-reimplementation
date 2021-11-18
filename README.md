# malloc-reimplementation
Reimplementation of Malloc and Free in C using mmap

El objetivo es reimplementar las conocidas funciones Malloc y Free utilizando mmap, cuya funcion es mapear una región a memoria y acceder a ella directamente como si fuera un array de bytes.

Para esta implementación se definio como tamaño del bloque el valor 16Kib, los cuales son allocados la primera vez que se llama a malloc y luego administrados (con la idea de una administracion eficiente) durante el resto del programa. Si se llama a free suficientes veces y la memoria libre vuelve a ser de 16Kib se llama a munmap para desallocar el espacio pedido. Si en algún momento llega un pedido de más memoria de la que queda disponible en el bloque el programa devuelve NULL.

Si bien tambien se definio 32Mib como pedido maximo que se le puede hacer a malloc, al ser el bloque de 16Kib esta condición no tiene mucho sentido. El tamaño minimo son 256 bytes por lo que aunque el usuario pida menos memoria, se le entregan igual 256 bytes.

La estrategia para devolver una region de memoria es first fit, es decir la primera región que se encuentra que tenga espacio suficiente se devuelve al usuario.

Un problema que podemos tener implementando un malloc y free es la extra fragmentacion debido a llamar a malloc muchas veces. En esta implementación ese problema busca solucionarse utilizando coalescing, es decir combinando los chunks frees con sus vecinos
si estos tambien han sido usados y liberados.
