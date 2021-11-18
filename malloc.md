
Parte 1: Administrador de bloques
--- 

El objetivo del challengue es reimplementar las conocidas funciones Malloc y Free utilizando mmap, cuya funcion es mapear una región a memoria y acceder a ella directamente como si fuera un array de bytes.

Para esta implementación se definio como tamaño del bloque el valor 16Kib, los cuales son allocados la primera vez que se llama a malloc y luego administrados (con la idea de una administracion eficiente) durante el resto del programa. Si se llama a free suficientes veces y la memoria libre vuelve a ser de 16Kib se llama a munmap para desallocar el espacio pedido. Si en algún momento llega un pedido de más memoria de la que queda disponible en el bloque el programa devuelve NULL.

Si bien tambien se definio 32Mib como pedido maximo que se le puede hacer a malloc, al ser el bloque de 16Kib esta condición no tiene mucho sentido. El tamaño minimo son 256 bytes por lo que aunque el usuario pida menos memoria, se le entregan igual 256 bytes.

La estrategia para devolver una region de memoria es first fit, es decir la primera región que se encuentra que tenga espacio suficiente se devuelve al usuario.

Un problema que podemos tener implementando un malloc y free es la extra fragmentacion debido a llamar a malloc muchas veces. En esta implementación ese problema busca solucionarse utilizando coalescing, es decir combinando los chunks frees con sus vecinos
si estos tambien han sido usados y liberados.

Se describe brevemente a continuación lo que ocurre al llamar a malloc:

La estructura del bloque esta compuesta de la siguiente manera: es una lista doblemente enlazada, en la que cada región conoce la dirección de la siguiente y de la anterior, en caso de existir. Se tiene el size de esa región, un valor int que indica si esta libre o ocupada, y un campo para la data, que es lo que se le devolvera al usuario para que escriba. Este campo data es necesario porque si nosotros le devolvieramos al usuario directamente un puntero al bloque, este sobreescribiria la metadata del bloque y ya no podriamos recuperarla, por eso directamente le damos este puntero a void que es data, para que utilice sin pisar la metadata.

Lo primero que se hace es alinear el tamaño pedido al siguiente multiplo de 4 mas cercano, luego se hacen chequeos que tienen que ver con el size pedido, que no supere el maximo y si es menor al minimo, se cambia por dicho minimo. Luego se verifica si es la primera vez que se llama a malloc, en caso de serlo se llama a la funcion extend heap:  Esta función pide memoria via mmap e inicializa el bloque de memoria, seteando los 16Kib como si fueran una unica región con la que ya podemos trabajar. 

Si no fuera la primera vez que se llama a malloc, se llama a la función find_block que recorre la lista enlazada de regiones hasta encontra una que cumpla dos condiciones: tener el espacio suficiente para el pedido del usuario y que el bloque este marcado como libre. Find_block recorre hasta encontrar un bloque valido y lo devuelve, pero si encuentra un bloque NULL, para evitar un seg fault directamente corta la ejecución y devuelve NULL. Esta iteración podria ejecutarse un maximo de TAM_BLOQUE/TAM_MINIMO veces, es decir 16Kib / 256 b = 64 veces. 64 es la cantidad Maxima de regiones que podria haber dado los tamaños elegidos.

Una vez se devolvio un bloque valido, ya sea por extend heap o por find block se debe llamar a split block, cuya función es cortar el size que el usuario no pidio para crear una nueva región que pueda ser devuelta la siguiente vez que se llame a malloc. Lo que hace split block es crear un new_block en la dirección old_block->data + size, esto asegura que el nuevo bloque este mapeado en memoria justo donde terminar el espacio de data del anterior, de esta manera el usuario no va pisar contenido de los bloques en la memoria. Se inicializa el nuevo bloque, colocando correctamente sus vecinos y el tamaño que tendra disponible.

Por ultimo se setea el bloque como ocupado y se devuelve al usuario el campo data del bloque, el cual ya puede usarlo como quiera. Se presenta un pseudocodigo de malloc:
```
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
```
Luego de utilizar el bloque de memoria el usuario puede llamar a free, pasandole un puntero a void. Este puntero justamente va a apuntar al campo data del bloque, entonces sabemos que justo antes de dicho puntero, en memoria, esta la metadata del bloque, con lo que podemos recuperar nuestra estructura del bloque. Con esto podemos validar que el bloque exista y que no haya sido liberado antes (doble free).

Necesitamos aplicar coalescing, por lo que revisamos si sus vecinos estan libres (antes revisamos si existen), en caso afirmativo debemos combinar nuestro bloque con su/s vecino/s. No solo debemos combinarlos en cuestion de tamaño sino que ademas debemos setear correctamente sus nuevos vecinos.

Una vez hecho esto, revisamos si el nuevo bloque (ya combinado con sus vecinos) tiene el tamaño del bloque completo, es decir 16Kib, si este fuera el caso, es que desallocamos todos los bloques pedidos por lo que usamos la funcion munmap y "deshacemos" el heap. Si vuelve a llamarse a malloc este heap volvera armarse mediante mmap.

Algo para destacar es que los errores en esta implementación, es decir double free e invalid free, simplemente se imprimen por pantalla y retorna, mientras que la version original de free corta la ejecución mediante una interrupción. Esto seria facilmente imitable reemplazando los return por, por ejemplo, la syscall exit(-1).

Se presenta pseudocodigo de free:
```
def free(p):
    if p is valid:
        obtenemos la direccion del bloqe
        lo marcamos como libre
        si el bloque previo existe y esta free, fusionamos
        tratamos tambien de fusionar el siguiente bloque
        si estamos en el ultimo bloque del heap, liberamos memoria
        si no hay mas bloques, seteamos base = null
    else:
        error
```


Testing
--- 

Para correr los tests: gcc -o testing testing.c mymalloc.c y luego ./testing

Se creo un programa de pruebas llamado testing.c que, si bien de manera algo artesanal, prueba el funcionamiento de la implementación de malloc y free. Desde ya podemos imaginarnos que si bien el funcionamiento de nuestro malloc debe ser similar al malloc de la libreria estandar, en cierto punto dicho fucionamiento va a diferir, sobretodo porque nuestra versión tiene la seria limitación de allocar maximo 16Kib en memoria, limitación que no existe para la version "original".

Veremos las pruebas una por una, y de existir diferencias con la implementación original de malloc las mismas seran descriptas.

El primer test es muy sencillo, simplemente se le pasa un objeto invalido, en este caso NULL y se espera que el programa imprima un invalid free.

El segundo test replica el mismo comportamiento 500 veces, que es el siguiente: alloca un size de sizeof(int) * 512, es decir 2048 bytes y llena este espacio con enteros, luego verifica que todos los enteros se hallan mapeado correctamente y libera la memoria pedida. Esta prueba, allocar y desallocar un unico bloque 500 veces, pone a prueba la creación y destrucción del heap, el cual se realiza en cada iteración ya que al llamarse a un unico malloc, siempre el heap debe crearse y luego se destruye con el free.

El tercer test primero alloca un int, es decir intenta allocar 4 bytes, pero si recordamos que el tamaño minimo son 256 bytes, es esta la cantidad de memoria que se le pide a malloc. Luego intentar allocarse una nueva estructrura que pesa 16Kib. Al exceder esta nueva estructura + el pedido anterior de 256 b el tamaño del bloque, esperamos que la implementación falle, es decir devuelva un NULL por falta de espacio. Este funcionamiento difiere del malloc de la libreria estandar, que podria allocar ambos pedidos sin ningun problema.

El test cuatro es muy sencillo, simplemente crea un vector de cadenas, y luego alloca tres cadenas con malloc, cada una de 256 bytes. Setea un texto (menor a 256 bytes) a cada cadena y luego la guarda en el vector. Luego de allocar las 3 cadenas las imprime en orden y luego las libera con free, sin resultar en ningun error.

El test cinco lo que hace es allocar primero dos cadenas de 256 bytes cada una, a y b. Luego libera a, a continuación intenta liberar a nuvamente y lo que se obtiene es un error del tipo double free, que es lo esperado por esta prueba. Si no hubiesemos allocado dos cadenas, sino una sola, y la hubiesemos liberado dos veces, el resultado hubiese sido exactamente el mismo.

El test seis hace lo siguiente: primero alloca un punteo a int de 256 bytes y luego trata de allocar una estructura de 16Kib, como ya se supera el tamaño maximo del bloque el programa falla y no alloca esta segunda estructura. Lo que sucede a continuación es que se desalloca la primera estructura y nuevamente se intenta alloca la segunda, mas pesada. La prueba finaliza con la segunda estructura allocada correctamente porque ahora si hay espacio suficiente. Con la versión de malloc de la libreria estandar, como ya sucedio antes, no hubieramos tenido problema en allocar las dos estructuras juntas.

La prueba número 7 tiene un funcionamiento similar a la seis pero busca probar el correcto funcionamiento de malloc y free mediante una secuencia de uso de los mismos que compruebe que el reciclaje de memoria funciona correctamente. Se hace lo siguiente: primero se allocan tres elementos, a de 4096 bytes, b de 6144 bytes y c de 4096 bytes. A continuación se intenta allocar una estructura d de 2048 bytes, esta estructura a priori podriamos pensar que deberia poder allocarse ya que si sumamos los 4 pesos obtenemos 16Kib que es el tamaño del bloque, sin embargo si recordamos que cada bloque tiene su metadata que por supuesto ocupa espacio no debemos soprendernos cuando esta estructura d no pueda allocarse. Lo que hacemos a continuación es liberar el bloque mas pesado, b de 6144 bytes. Nuevamente intenamos allocar d, el cual ahora si sera allocado en el espacio que dejo libre b, teniendo como vecino previo a "a" y como vecino siguiente un bloque vacio justo antes de c. En ese espacio vacio guardamos otra estructura de 2048 bytes, llamada e, la cual tendra como vecinos a d y otro nuevo bloque vacio, mas pequeño, antes de c. Luego se borran mediante free los 4 bloques sin ningun error. Con la versión de malloc de la libreria estandar, como ya sucedio antes, no hubieramos tenido problema en allocar todas las estructuras juntas.

Si miramos todas las pruebas, vemos que en cada una de ellas de desallocan todos los elementos allocados, es decir que al final de cada una se destruye nuestro heap por
lo que cada prueba es como si corrieramos un programa individual para ella. Para verificar un poco la robustez del programa se hizo lo siguiente: se alloco antes de llamar a las pruebas una cadena de 256 bytes, la cual se desalloco recien luego de correr todas las pruebas, esto genero que el heap no se destruyera en ningun momento despues del malloc de la cadena. Los resultados de esto fueron que todas las pruebas pasan exitosamente, salvo algunas donde se allocaban estructuras de 16Kib, las cuales logicamente ahora no podrian allocarse ya que no entrarian por espacio, pero ese comportamiento tambien es el esperado. El contenido de la cadena que se definio antes de las pruebas se imprimio al final de las mismas y se observo exitosamente que dicho contenido no cambio.
