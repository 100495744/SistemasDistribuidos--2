//Este es el main que hace pruebas con las funciones
//Se supone que esto usa la librería dinámica

/* USA ESTOS COMANDOS

# Compilar la biblioteca dinámica
gcc -shared -o libclaves.so -fPIC proxy-mq.c -lrt

# Compilar el servidor
gcc -o servidor-mq servidor-mq.c

# Compilar el cliente
gcc -o app-cliente app-cliente.c -L. -lclaves -lrt

# Establecer la variable de entorno
export LD_LIBRARY_PATH=.

# Ejecutar el servidor (en una terminal)
./servidor-mq

# Ejecutar el cliente (en otra terminal)
./app-cliente

*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../claves.h"
#include<unistd.h>

void test_set_value() {
    int key = 1;
    char value1[256] = "HolaSoyLaPrimeraClave";
    int N_value2 = 3;
    double V_value2[3] = {1.0, 2.0, 3.0};
    struct Coord value3 = {1, 2};

    if ( set_value(key, value1, N_value2, V_value2, value3) == -1){
        printf("test set value1 error\n");
        return NULL;
    }
    return NULL;
}


void test_set_value2(int i) {
    int key = i;
    char value1[256] = "HolaSoyLaSegundaClave";
    int N_value2 = 3;
    double V_value2[3] = {6.0, 2.0, 3.0};
    struct Coord value3 = {1, 2};

    if ( set_value(key, value1, N_value2, V_value2, value3) == -1){
        printf("test_set_value 2 error\n");
        return NULL;
    }
    return NULL;
}

void test_get_value() {
    int key = 1;
    char value1[256];
    int N_value2;
    double V_value2[32];
    struct Coord value3;

    if ( get_value(key, value1, &N_value2, V_value2, &value3) == -1){
        printf("test get value 1 error\n");
        return NULL;
    }
    printf("Mensaje del cliente: El value1 recibido es: %s\n", value1);
    printf("Mensaje del cliente: El N_value2 recibido es: %d\n", N_value2);
    for (int i = 0; i < N_value2; i++) {
        printf("Mensaje del cliente: El V_value2[%d] recibido es: %f\n", i, V_value2[i]);
    }
    printf("Mensaje del cliente: El value3 recibido es: (%d, %d)\n", value3.x, value3.y);
    return NULL;
}

void test_modify_value() {
    int key = 1;
    char value1[256] = "HolaHeCambiado";
    int N_value2 = 2;
    double V_value2[2] = {4.0, 5.0};
    struct Coord value3 = {6, 7};
    if (modify_value(key, value1, N_value2, V_value2, value3) == -1){
        printf("Test modify 1 error\n");
        return NULL;
    };
    return NULL;
}

void test_delete_key() {
    int key = 1;

    if (delete_key(key) == -1){
        perror("Error al deletear la Key\n ");
        return NULL;
    }
    return NULL;
}

void test_destroy() {
    if ( destroy() == -1){
        perror("error al reiniciar la base de datos");
        return NULL;
    }
    return NULL;
}

int main() {
    sleep(1);
    test_get_value();
    test_set_value();
    test_get_value();
    test_modify_value();
    test_get_value();
    for (int i= 0; i < 100 ; i++){
        test_set_value2(i);
    }


    return 0;
}