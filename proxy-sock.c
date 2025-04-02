#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <mqueue.h>
#include <dirent.h>
#include <errno.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include "claves.h"
#include "lines.h"

struct peticion{
    int op; // Operación a realizar
    int key;
    char value1[256];
    int N_value2;
    double V_value2[32];
    struct Coord value3;
};

struct respuesta{
    int key;
    char value1[256];
    int N_value2;
    double V_value2[32];
    struct Coord value3;
    int status; // Éxito o fracaso
};

typedef enum {
    OP_DESTROY = 0,
    OP_EXIST,
    OP_GET,
    OP_SET,
    OP_MODIFY,
    OP_DELETE
} Operacion;

int make_conection(){
    printf("-1");
    char *IP_SERVER = getenv("IP_TUPLAS");
    if (IP_SERVER == NULL){
        printf("Variable IP_TUPLAS no definida\n");
        return -1;
    }
    printf("-2");
    char *PORT_SERVER = getenv("PORT_TUPLAS");
    if (PORT_SERVER == NULL){
        printf("Variable PORT_TUPLAS no definida\n");
        return -1;
    }
    printf("1");
    short puerto = (short) atoi(PORT_SERVER);
    int sd;
    struct sockaddr_in server_addr;
    struct hostent *hp;
    printf("2");
    sd = socket(AF_INET, SOCK_STREAM, 0);
    printf("3");
    if (sd == 1) {
        printf("Error en socket\n");
        close(sd);
        return -1;
    }

    printf("4");
    bzero((char *)&server_addr, sizeof(server_addr));
    hp = gethostbyname(IP_SERVER);
    printf("5");
    if (hp == NULL) {
        close(sd);
        printf("Error en gethostbyname\n");
        return -1;
    }

    printf("6");
    memcpy (&(server_addr.sin_addr), hp->h_addr, hp->h_length);
    server_addr.sin_family  = AF_INET;
    server_addr.sin_port    = htons(puerto);

    printf("7");
    int err = connect(sd, (struct sockaddr *) &server_addr,  sizeof(server_addr));
    if (err == -1) {
        close(sd);
        printf("Error en connect\n");
        return -1;
    }

    return sd;
}


int enviar_peticion( int sc , struct peticion *pet){
    if (sc == 0 ){
        printf("Error in connection");
    }

    int op  = htonl(pet->op); // Operación a realizar
    int key = htonl(pet->key);
    char value1[256];

    strcpy(value1, pet->value1);
    int N_value2 = htonl(pet->N_value2);
    double V_value2[32];
    for ( int i = 0; i < N_value2; i++){
        V_value2[i] = htonl(pet->V_value2[i]);
    }
    int V_value3_x = htonl(pet->value3.x);
    int V_value3_y = htonl(pet->value3.y);



    int err = sendMessage( sc, (char *) &op, sizeof(int));   // recibe la operació
    if (err == -1) {
        printf("Error en recepcion de op\n");
        close(sc);
        return -1;
    }


    err = sendMessage( sc, (char *) &key, sizeof(int));   // recibe la operació
    if (err == -1) {
        printf("Error en recepcion de value\n");
        close(sc);
        return -1;
    }


    err = sendMessage ( sc, (char *) value1, sizeof(pet->value1));   // recibe la operació
    if (err == -1) {
        printf("Error en recepcion de value\n");
        close(sc);
        return -1;
    }

    err = sendMessage ( sc, (char *) &N_value2, sizeof(int));   // recibe la operació
    if (err == -1) {
        printf("Error en recepcion de value\n");
        close(sc);
        return -1;
    }


    for ( int i = 0; i < pet->N_value2 ; i++){
        err = sendMessage ( sc, (char *)&V_value2[i], sizeof(double));   // recibe la operació
        if (err == -1) {
            printf("Error en recepcion de value\n");
            close(sc);
            return -1;
        }

    }


    err = sendMessage ( sc, (char *) &V_value3_x, sizeof(int));   // recibe la operació
    if (err == -1) {
        printf("Error en recepcion de value\n");
        close(sc);
        return -1;
    }


    err = sendMessage ( sc, (char *) &V_value3_y, sizeof(int));   // recibe la operació
    if (err == -1) {
        printf("Error en recepcion de value\n");
        close(sc);
        return -1;
    }

    return 1;


}


int recivir_respuesta(int sc ,struct respuesta *resp){
    int key;
    char value1[256];
    int N_value2;
    double V_value2;
    int value3;
    int status; // Éxito o fracaso

    int err = recvMessage(sc, (char *)&key, sizeof(int));  // envía el resultado
    if (err == -1) {
        printf("Error en envio\n");
        close(sc);
        return -1;
    }
    resp->key = ntohl(key);

    err = recvMessage(sc, (char *)&value1, sizeof(value1));  // envía el resultado
    if (err == -1) {
        printf("Error en envio\n");
        close(sc);
        return -1;
    }
    strcpy(resp->value1, value1);

    err = recvMessage(sc, (char *)&N_value2, sizeof(int));  // envía el resultado
    if (err == -1) {
        printf("Error en envio\n");
        close(sc);
        return -1;
    }
    resp->N_value2 = ntohl(N_value2);

    for ( int i = 0; i < resp->N_value2 ; i++){
    err = recvMessage(sc, (char *)&V_value2, sizeof(double));  // envía el resultado
    if (err == -1) {
        printf("Error en envio\n");
        close(sc);
        return -1;
    }
    resp->V_value2[i] = ntohl(V_value2);
    }


    err = recvMessage(sc, (char *)&value3, sizeof(int));  // envía el resultado
    if (err == -1) {
        printf("Error en envio\n");
        close(sc);
        return -1;
    }
    resp->value3.x = ntohl(value3);

    err = recvMessage(sc, (char *)&value3, sizeof(int));  // envía el resultado
    if (err == -1) {
        printf("Error en envio\n");
        close(sc);
        return -1;
        }
    resp->value3.y = ntohl(value3);


    err = recvMessage(sc, (char *)&status, sizeof(int));  // envía el resultado
    if (err == -1) {
        printf("Error en envio\n");
        close(sc);
        return -1;
    }
    resp->status = ntohl(status);
    close(sc);
    return 1;

}

int destroy() {
    printf("Mensaje del proxy: Enviando una petición destroy\n");
    int sc = make_conection();
    struct peticion peticion ;
    struct respuesta respuesta;


    if (sc == -1){
        return -1;
    }
    peticion.op = OP_DESTROY;
    if (enviar_peticion(sc , &peticion) == -1){
        close(sc);
        exit(-1);
    }
    if ( recivir_respuesta(sc, &respuesta) == -1 ){
        close(sc);
        exit(-1);
    }
    printf("respueta: %d\n", respuesta.status);

    return respuesta.status;
}

int set_value(int key, char *value1, int N_value2, double *V_value2, struct Coord value3) {
    printf("Mensaje del proxy: Enviando una petición set_value\n");
    struct peticion peticion;
    struct respuesta respuesta;
    printf("hola\n");

    peticion.op = OP_SET;
    peticion.key = key;
    strcpy(peticion.value1, value1);
    peticion.N_value2 = N_value2;
    memcpy(peticion.V_value2, V_value2, sizeof(double) * N_value2);
    peticion.value3 = value3;

    int sc = make_conection();
    enviar_peticion(sc , &peticion);
    recivir_respuesta(sc, &respuesta);

    close(sc);

    return respuesta.status;
}

int get_value(int key, char *value1, int *N_value2, double *V_value2, struct Coord *value3) {
    printf("Mensaje del proxy: Enviando una petición get_value\n");
    struct peticion peticion;
    struct respuesta respuesta;
    char qr_name[1024];

    peticion.op = OP_GET;
    peticion.key = key;

    int sc = make_conection();
    if (sc == -1){
        return -1;
    }
    enviar_peticion(sc , &peticion);
    recivir_respuesta(sc, &respuesta);

    close(sc);

    if (respuesta.status == 0){
        strcpy(value1, respuesta.value1);
        *N_value2 = respuesta.N_value2;
        memcpy(V_value2, respuesta.V_value2, sizeof(double) * (*N_value2));
        *value3 = respuesta.value3;
    }

    return respuesta.status;
}

int modify_value(int key, char *value1, int N_value2, double *V_value2, struct Coord value3) {
    printf("Mensaje del proxy: Enviando una petición modify_value\n");
    struct peticion peticion;
    struct respuesta respuesta;



    peticion.op = OP_MODIFY;
    peticion.key = key;
    strcpy(peticion.value1, value1);
    peticion.N_value2 = N_value2;
    memcpy(peticion.V_value2, V_value2, sizeof(double) * N_value2);
    peticion.value3 = value3;


    int sc = make_conection();
    enviar_peticion(sc , &peticion);
    recivir_respuesta(sc, &respuesta);

    close(sc);

    return respuesta.status;
}

int delete_key(int key) {
    printf("Mensaje del proxy: Enviando una petición delete_key\n");
    struct peticion peticion;
    struct respuesta respuesta;



    peticion.op = OP_DELETE;
    peticion.key = key;


    int sc = make_conection();
    enviar_peticion(sc , &peticion);
    recivir_respuesta(sc, &respuesta);

    close(sc);

    return respuesta.status;
}

int exist(int key) {
    printf("Mensaje del proxy: Enviando una petición exist\n");
    struct peticion peticion;
    struct respuesta respuesta;


    peticion.op = OP_EXIST;
    peticion.key = key;

    int sc = make_conection();
    enviar_peticion(sc , &peticion);
    recivir_respuesta(sc, &respuesta);

    close(sc);

    return respuesta.status;
}
