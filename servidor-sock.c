/* En este archivo deben encontrarse las comunicaciones con el cliente */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/socket.h>
#include <mqueue.h>
#include <pthread.h>
#include <dirent.h>
#include <errno.h>
#include "claves.h"
#include "claves.c"

pthread_mutex_t mutex;
pthread_cond_t hecho;
int creado = 0;
#define NUM_THREADS 10



typedef struct {
    int op; //Operación a realizar
    int key;
    char value1[256];
    int N_value2;
    double V_value2[32];
    struct Coord value3;
}peticion;

typedef struct {
    int key;
    char value1[256];
    int N_value2;
    double V_value2[32];
    struct Coord value3;
    int status; // Éxito o fracaso
}respuesta;


//Esto es simplemente para que el switch quede más limpio. Asigna nombres a las operaciones numéricas
typedef enum {
    OP_DESTROY = 0,
    OP_EXIST,
    OP_GET,
    OP_SET,
    OP_MODIFY,
    OP_DELETE
} Operacion;

struct mq_attr attr_peticion;
struct mq_attr attr_respuesta;

void inicializar_attr() {
    //cola de peticiones
    attr_peticion.mq_flags = 0;                
    attr_peticion.mq_maxmsg = 10;              // Máximo número de mensajes en la cola
    attr_peticion.mq_msgsize = sizeof(peticion); // Tamaño de los mensajes de peticiones

    //cola de respuestas
    attr_respuesta.mq_flags = 0;               
    attr_respuesta.mq_maxmsg = 10;             // Máximo número de mensajes en la cola
    attr_respuesta.mq_msgsize = sizeof(respuesta); // Tamaño de los mensajes de respuestas
}

peticion coger_datos_peticion(int *sc){
    //coger los datos de la petición
    err = recvMessage ( sc, (int *) &pet->op, sizeof(int));   // recibe la operació
    if (err == -1) {
        printf("Error en recepcion de op\n");
        close(sc);
        continue;
    }
    pet.op = ntohl(pet.op);

    err = recvMessage ( sc, (int *) &pet->key, sizeof(int));   // recibe la operació
    if (err == -1) {
        printf("Error en recepcion de value\n");
        close(sc);
        continue;
    }
    pet.key = ntohl(pet.key);
    err = recvMessage ( sc, (char *) &pet->value1, sizeof(pet->value1));   // recibe la operació
    if (err == -1) {
        printf("Error en recepcion de value\n");
        close(sc);
        continue;
    }
    err = recvMessage ( sc, (int *) &pet->N_value2, sizeof(int));   // recibe la operació
    if (err == -1) {
        printf("Error en recepcion de value\n");
        close(sc);
        continue;
    }
    pet.N_value2 = ntohl(pet.Nvalue2);

    for ( i = 0; i < pet->N:value2 , i++){
        err = recvMessage ( sc, (double *) &pet->V_value2[i], sizeof(double));   // recibe la operació
        if (err == -1) {
            printf("Error en recepcion de value\n");
            close(sc);
            continue;
        }
        pet.V_value2[i] = ntohl(pet.V_value2[i]);
    }


    err = recvMessage ( sc, (coord *) &pet->value3, sizeof(struct Coord));   // recibe la operació
    if (err == -1) {
        printf("Error en recepcion de value\n");
        close(sc);
        continue;
    }

    return pet;
}

void tratar_peticion(int  *soquet_cliente){
    //asegurarnos que se crea el hilo
    pthread_mutex_lock(&mutex);
    printf("hilo creado\n");
    int sd = *soquet_cliente;
    creado = 1;
    pthread_cond_signal(&hecho);
    pthread_mutex_unlock(&mutex);

    respuesta respuesta;
    peticion pet;
    int prio = 0;
    inicializar_attr();

    pet = coger_datos_peticion(&sd)

    switch(pet.op){

        case OP_DESTROY: //No se si destroy es un caso válido
            printf("He recibido una petición de destrucción\n");
            pthread_mutex_lock(&mutex);
            respuesta.status = destroy();
            pthread_mutex_unlock(&mutex);
            if (respuesta.status == 0){
                printf("Base de datos destruida\n");
            }
            else{
                printf("Error al destruir la base de datos\n");
            }
            break;

        case OP_EXIST:
            printf("He recibido una petición de existencia\n");
            pthread_mutex_lock(&mutex);
            int existe = exist(pet.key);
            pthread_mutex_unlock(&mutex);
            if (existe == 1){
                printf("La clave existe\n");
            }
            else if (existe == 0){
                printf("La clave no existe\n");
            }
            else{
                printf("Error al comprobar la existencia de la clave\n");
                respuesta.status = -1;
            }
            respuesta.status = 0;
            break;

        case OP_GET:
            printf("He recibido una petición de get\n");
            pthread_mutex_lock(&mutex);
            respuesta.status = get_value(pet.key, respuesta.value1, &respuesta.N_value2, respuesta.V_value2, &respuesta.value3);
            pthread_mutex_unlock(&mutex);
            //DEBUG
            /*printf("respuesta.status = %d\n", respuesta.status);
            printf("respuesta.value1 = %s\n", respuesta.value1);
            printf("respuesta.N_value2 = %d\n", respuesta.N_value2);
            for (int i = 0; i < respuesta.N_value2; i++){
                printf("respuesta.V_value2[%d] = %f\n", i, respuesta.V_value2[i]);
            }
            printf("respuesta.value3 = (%d, %d)\n", respuesta.value3.x, respuesta.value3.y);*/

            if (respuesta.status == 0){
                printf("Valor obtenido correctamente\n");
            }
            else{
                printf("Error al obtener el valor\n");
            }
            break;

        case OP_SET:
            printf("He recibido una petición de set\n");
            pthread_mutex_lock(&mutex);
            respuesta.status = set_value(pet.key, pet.value1, pet.N_value2, pet.V_value2, pet.value3);
            pthread_mutex_unlock(&mutex);
            //printf("respuesta status = %d\n", respuesta.status);
            if (respuesta.status == 0){
                printf("Valor insertado correctamente\n");
            }
            else{
                printf("Error al insertar el valor\n");
            }
            break;

        case OP_MODIFY:
            printf("He recibido una petición de modify\n");
            pthread_mutex_lock(&mutex);
            respuesta.status = modify_value(pet.key, pet.value1, pet.N_value2, pet.V_value2, pet.value3);
            pthread_mutex_unlock(&mutex);
            if (respuesta.status == 0){
                printf("Valor modificado correctamente\n");
            }
            else{
                printf("Error al modificar el valor\n");
            }
            break;

        case OP_DELETE:
            printf("He recibido una petición de delete\n");
            pthread_mutex_lock(&mutex);
            respuesta.status = delete_key(pet.key);
            pthread_mutex_unlock(&mutex);
            if (respuesta.status == 0){
                printf("Clave eliminada correctamente\n");
            }
            else{
                printf("Error al eliminar la clave\n");
            }
            break;

        default:
            printf("Operación no válida\n");
            respuesta.status = -1;
            break;
    }

    pthread_mutex_lock(&mutex);
    int qr = mq_open(pet.q_name, O_CREAT | O_WRONLY, 0700, &attr_respuesta);
    if (qr == -1) {
        perror("Error en mq_open del servidor");
        return -1;
    }
    if (mq_send(qr, (char *)&respuesta, sizeof(respuesta), 0) == -1) {
        perror("Error en mq_send del servidor");
        return -1;
    }
    mq_close(qr);
    pthread_mutex_unlock(&mutex);

    err = sendMessage(sc, (char *)&res, sizeof(int32_t));  // envía el resultado
    if (err == -1) {
        printf("Error en envi­o\n");
        close(sc);
        continue;
    }

    close(sc);
}


int main(int argc , char **argv){
    //Argumentos validos
    if (argc < 2 ){
        printf("Número de argumentos invalido");
        return -1;
    }

    //Preparación de la base de datos
    DIR *Dir;
    Dir = opendir(DirName);
    if ( Dir == NULL){
        mkdir("DataBase",S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
    }
    //codigo de soquets
    struct sockaddr_in server_addr,  client_addr;
    socklen_t size;
    int sd;
    int socket_number = atoi(argv[1])

    if ((sd =  socket(AF_INET, SOCK_STREAM, 0))<0){
        printf ("SERVER: Error en el socket");
        return (0);
    }

    bzero((char *)&server_addr, sizeof(server_addr));
    server_addr.sin_family      = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port        = htons(socket_number);
    val = 1;
    setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, (char *) &val, sizeof(int));
    //binding
    err = bind(sd, (const struct sockaddr *)&server_addr,
               sizeof(server_addr));
    if (err == -1) {
        printf("Error en bind\n");
        return -1;
    }

    respuesta respuesta;
    peticion peticion;
    int prio;
    mq_unlink("/SERVIDOR");
    pthread_attr_t attr;
    pthread_t thr;
    
    inicializar_attr();
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);


    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&hecho, NULL);


    printf("############## SERVIDOR CONECTADO ##############\n");
    printf("Esperando peticiones...\n");

    //server start listening
    err = listen(sd, SOMAXCONN);
    if (err == -1) {
        printf("Error en listen\n");
        return -1;
    }

    size = sizeof(client_addr);
    while (1){
        //we check that the Database has not been deleted while the server is running
        Dir = opendir(DirName);
        if ( Dir == NULL){
            mkdir("DataBase",S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
        }

        //accept conexión de un cliente
        printf("esperando conexion\n");
        sc = accept(sd, (struct sockaddr *)&client_addr, (socklen_t *)&size);

        if (sc == -1) {
            printf("Error en accept\n");
            return -1;
        }


        //creación del hilo
        if(pthread_create(&thr, &attr, (void *) tratar_peticion, &sc) == -1){
            perror("Error al crear hilo");
            return -1;
        }
        pthread_mutex_lock(&mutex);
        while (creado == 0){
            pthread_cond_wait(&hecho,&mutex);
        }
        creado = 0;
        pthread_mutex_unlock(&mutex);
    };

    mq_close(qs);
    mq_unlink("/SERVIDOR");
    return 0;
}

