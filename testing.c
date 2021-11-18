#include <stdio.h>
#include <string.h>
#include "mymalloc.h"

static void test1() {
    printf("TEST 1 --- Expected: Invalid Free. Result: ");
    myfree(NULL);
    printf("TEST 1 Completo\n\n");
}

static void test2() {
    printf("TEST 2 --- El test deberia finalizar sin resultados inesperados\n");
    for (int i = 0; i < 500; i++){
        int *a = mymalloc(sizeof(int) * 512);
        if (a == NULL) {
            printf("ERROR: Resultado Inesperado!\n");
            return;
        }        
        for(int j = 0; j < 512; j ++)
            a[j]= j;
        for (int j = 0; j < 512; j ++) {
            if (a[j] != j) {   
                printf("ERROR: Resultado Inesperado!\n");
                return;
            }
        }
        myfree(a);
    }
    printf("TEST 2 Completo\n\n");
}

static void test3() {
    printf("TEST 3 --- El test deberia finalizar sin poder allocar A por falta de espacio:\n");
    int *b = mymalloc(sizeof(int));
    int *a = mymalloc(sizeof(int) * 1024*4);
    if (a == NULL){
         printf("No se pudo allocar A\n");
    } else {
        printf("ERROR, no deberia haberse allocado A\n");
        myfree(a);
    }
    myfree(b);
    printf("TEST 3 Completo\n\n");
}

static void test4() {
    printf("TEST 4 --- El test deberia imprimir las 3 cadenas de forma ordenada y no fallar:\n");

    char** vector[3];

    char *a = mymalloc(256);
    if (a == NULL) 
        printf("ERROR: Fallo malloc con A\n");
    strcpy(a,"cadena A");
    vector[0] = &a;
    
    char *b = mymalloc(256);
    if (b == NULL) 
        printf("ERROR: Fallo malloc con B\n");
    strcpy(b,"cadena B");
    vector[1] = &b;
    
    char *c = mymalloc(256);
    if (c == NULL) 
        printf("ERROR: Fallo malloc con C\n");    
    strcpy(c,"cadena C");
    vector[2] = &c;

    printf("a: %s\n",*vector[0]);
    printf("b: %s\n",*vector[1]);
    printf("c: %s\n",*vector[2]);

    myfree(b);
    myfree(c);
    myfree(a);  

    printf("TEST 4 Completo\n\n");
}

static void test5() {
    printf("TEST 5 --- Expected: Double Free. Result: ");
    char *a = mymalloc(256);
    char *b = mymalloc(256);
    myfree(a);
    myfree(a);
    myfree(b);

    printf("TEST 5 Completo\n\n");
}

static void test6() {
    printf("TEST 6 --- Se espera que no pueda allocar a luego de allocar b, pero si pueda luego de desallocar b\n");
    printf("Alloco B\n");
    int *b = mymalloc(sizeof(int));
    printf("Alloco A\n");
    int *a = mymalloc(sizeof(int) * 1024*4);
    if (a == NULL){
         printf("No se pudo allocar A\n");
    } else {
        printf("ERROR, no deberia haberse allocado A\n");
        myfree(a);
    }
    printf("Desalloco B\n");
    myfree(b);
    printf("Alloco A nuevamente\n");
    a = mymalloc(sizeof(int) * 1024*4);
    if (a == NULL){
         printf("ERROR: No se pudo allocar A\n");
    } else {
        printf("A se alloco correctamente\n");
        myfree(a);
    }
    printf("TEST 6 Completo\n\n");
}

static void test7() {
    printf("TEST 7 --- Alloco elementos hasta llenar mi heap, borro un elemento del medio y agrego elementos alli\n");
    char *a = mymalloc(4096);
    if (a == NULL) {
        printf("No se pudo alocar el elemento A\n");
    } 
    char *b = mymalloc(6144);
    if (b == NULL) {
        printf("No se pudo alocar el elemento B\n");
    } 
    char *c = mymalloc(4096);
    if (c == NULL) {
        printf("No se pudo alocar el elemento C\n");
    }         
    char *d = mymalloc(2048);
    if (d == NULL) {
        printf("No se pudo alocar el elemento D\n");
    }
    myfree(b);

    d = mymalloc(2048);
    if (d == NULL) {
        printf("No se pudo alocar el elemento D\n");
    } else {
        printf("Libere B y pude allocar D\n");
    }

    char *e = mymalloc(2048);
    if (e == NULL) {
        printf("No se pudo alocar el elemento e\n");
    } else {
        printf("pude allocar E\n");
    }

    myfree(a);
    myfree(c);
    myfree(d);
    myfree(e);

    printf("TEST 7 Completo\n\n");
}

int main() {
  //  char* cadena = mymalloc(256);
  //  strcpy(cadena,"hola mundo");
 
    test1();
    test2();
    test3();
    test4();
    test5();
    test6();
    test7();

   // printf("cadena final: %s\n",cadena);
   // myfree(cadena);
    return 0;
}