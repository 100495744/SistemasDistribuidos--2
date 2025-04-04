/* En este archivo deben encontrarse las comunicaciones con el cliente */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <mqueue.h>
#include <pthread.h>
#include <dirent.h>
#include <errno.h>
#include "claves.h"
#include "lines.h"

pthread_mutex_t mutex;
pthread_cond_t hecho;
int creado = 0;
#define NUM_THREADS 10
#define DirName "DataBase"



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

int coger_datos_peticion(int sc, peticion *pet ){
    //coger los datos de la petición


    int err = recvMessage ( sc, (char *) &pet->op, sizeof(int));   // recibe la operació
    if (err == -1) {
        printf("Error en recepcion de op\n");
        close(sc);
        return -1;
    }
    pet->op = ntohl(pet->op);

    err = recvMessage( sc, (char *) &pet->key, sizeof(int));   // recibe la operació
    if (err == -1) {
        printf("Error en recepcion de value\n");
        close(sc);
        return -1;
    }
    pet->key = ntohl(pet->key);

    err = recvMessage ( sc, (char *) &pet->value1, sizeof(char[256]));   // recibe la operació
    if (err == -1) {
        printf("Error en recepcion de value\n");
        close(sc);
        return -1;
    }


    err = recvMessage ( sc, (char *) &pet->N_value2, sizeof(int));   // recibe la operació
    if (err == -1) {
        printf("Error en recepcion de value\n");
        close(sc);
        return -1;
    }

    pet->N_value2 = ntohl(pet->N_value2);


    for ( int i = 0; i < pet->N_value2 ; i++){
        err = recvMessage ( sc, (char *)&pet->V_value2[i], sizeof(double));   // recibe la operació
        if (err == -1) {
            printf("Error en recepcion de value\n");
            close(sc);
            return -1;
        }
        pet->V_value2[i] = ntohl(pet->V_value2[i]);
    }


    err = recvMessage ( sc, (char *) &pet->value3.x, sizeof(int));   // recibe la operació
    if (err == -1) {
        printf("Error en recepcion de value\n");
        close(sc);
        return -1;
    }

    pet->value3.x = ntohl(pet->value3.x);

    err = recvMessage ( sc, (char *) &pet->value3.y, sizeof(int));   // recibe la operació
    if (err == -1) {
        printf("Error en recepcion de value\n");
        close(sc);
        return -1;
    }
    pet->value3.y = ntohl(pet->value3.y);

    return 0;
}

//Mandar la respuesta al cliente
int mandar_respuesta(respuesta *res ,int sc ){
    int key = htonl(res->key);
    int N_value2= htonl(res->N_value2);
    double V_value2[32];
    for (int i = 0; i < res->N_value2; i++) {
        V_value2[i] = htonl(res->V_value2[i]);
    }

    int value3x = htonl(res->value3.x);
    int value3y = htonl(res->value3.y);
    int status = htonl(res->status);

    int err;


    err = sendMessage(sc, (char *)&status, sizeof(int));  // envía el resultado
    if (err == -1) {
        printf("Error en envio\n");
        close(sc);
        return -1;
    }

    if ( res->status == -1 ){
        printf("Solo estatus mandado por error \n");
        close(sc);
        return 0;
    }

    err = sendMessage(sc, (char *)&key, sizeof(int));  // envía el resultado
    if (err == -1) {
        printf("Error en envio\n");
        close(sc);
        return -1;
    }

    err = sendMessage(sc, res->value1, sizeof(char[256]));  // envía el resultado
    if (err == -1) {
        printf("Error en envio\n");
        close(sc);
        return -1;
    }

    err = sendMessage(sc, (char *)&N_value2, sizeof(int));  // envía el resultado
    if (err == -1) {
        printf("Error en envio\n");
        close(sc);
        return -1;
    }

    for( int i =0; i < res->N_value2; i++){
        printf("%d\n", i);
    err = sendMessage(sc, (char *)&V_value2[i], sizeof(double));  // envía el resultado
    if (err == -1) {
        printf("Error en envio\n");
        close(sc);
        return -1;
        }
    }
    err = sendMessage(sc, (char *)&value3x, sizeof(int));  // envía el resultado
    if (err == -1) {
        printf("Error en envio\n");
        close(sc);
        return -1;
    }
    err = sendMessage(sc, (char *)&value3y, sizeof(int));  // envía el resultado
    if (err == -1) {
        printf("Error en envio\n");
        close(sc);
        return -1;
    }
    printf("petición enviada\n");
    return 0;
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

    if ( coger_datos_peticion(sd, (peticion *) &pet) == -1){
        printf("error coger datos");
        exit(-1);
    }


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
                respuesta.status = 1;
            }
            else if (existe == 0){
                printf("La clave no existe\n");
                respuesta.status = 0;
            }
            else{
                printf("Error al comprobar la existencia de la clave\n");
                respuesta.status = -1;
            }

            break;

        case OP_GET:
            printf("He recibido una petición de get con key: %d\n", pet.key);
            pthread_mutex_lock(&mutex);
            respuesta.status = get_value(pet.key, respuesta.value1, &respuesta.N_value2, respuesta.V_value2, &respuesta.value3);
            pthread_mutex_unlock(&mutex);

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
    printf("Mandando respesta\n");
   if ( mandar_respuesta( &respuesta , sd ) == -1) {
        printf("Error mandar respuesta\n");
       exit(-1);
   }
   close(sd);
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
    int socket_number = atoi(argv[1]);


    if ((sd =  socket(AF_INET, SOCK_STREAM, 0))<0){
        printf ("SERVER: Error en el socket");
        return (0);
    }
    int val = 1;

    bzero((char *)&server_addr, sizeof(server_addr));
    server_addr.sin_family      = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port        = htons(socket_number);

    setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, (char *) &val, sizeof(int));
    //binding
    int err = bind(sd, (const struct sockaddr *)&server_addr,
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
        int sc = accept(sd, (struct sockaddr *)&client_addr, (socklen_t *)&size);

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

   close(sd);
   return 0;
}

