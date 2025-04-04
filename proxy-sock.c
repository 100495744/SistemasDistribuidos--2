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


int op; // Operación a realizar
int key = 0;
char *value1[256] ;
int N_value2 = 0;
double V_value2[32];
struct Coord value3 = (struct Coord) {0,0};

int status;

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

    char *IP_SERVER = getenv("IP_TUPLAS");
    if (IP_SERVER == NULL){
        printf("Variable IP_TUPLAS no definida\n");
        return -1;
    }

    char *PORT_SERVER = getenv("PORT_TUPLAS");
    if (PORT_SERVER == NULL){
        printf("Variable PORT_TUPLAS no definida\n");
        return -1;
    }

    short puerto = (short) atoi(PORT_SERVER);
    int sd;
    struct sockaddr_in server_addr;
    struct hostent *hp;

    sd = socket(AF_INET, SOCK_STREAM, 0);

    if (sd == 1) {
        printf("Error en socket\n");
        close(sd);
        return -1;
    }


    bzero((char *)&server_addr, sizeof(server_addr));
    hp = gethostbyname(IP_SERVER);

    if (hp == NULL) {
        close(sd);
        printf("Error en gethostbyname\n");
        return -1;
    }


    memcpy (&(server_addr.sin_addr), hp->h_addr, hp->h_length);
    server_addr.sin_family  = AF_INET;
    server_addr.sin_port    = htons(puerto);


    int err = connect(sd, (struct sockaddr *) &server_addr,  sizeof(server_addr));
    if (err == -1) {
        close(sd);
        printf("Error en connect\n");
        return -1;
    }

    return sd;
}


int enviar_peticion( int sc ){



    if (sc == 0 ){
        printf("Error in connection");
    }

    int op_l  = htonl( op ); // Operación a realizar
    int key_l = htonl(key);
    char value1_l[256];

    strcpy(value1_l, value1);
    int N_value2_l = htonl(N_value2);
    double V_value2_l[32];
    for ( int i = 0; i < N_value2; i++){
        V_value2_l[i] = htonl(V_value2[i]);
    }
    int V_value3_x = htonl(value3.x);
    int V_value3_y = htonl(value3.y);



    int err = sendMessage( sc, (char *) &op_l, sizeof(int));   // recibe la operació
    if (err == -1) {
        printf("Error en recepcion de op\n");
        close(sc);
        return -1;
    }


    err = sendMessage( sc, (char *) &key_l, sizeof(int));   // recibe la operació
    if (err == -1) {
        printf("Error en recepcion de value1\n");
        close(sc);
        return -1;
    }


    err = sendMessage ( sc, (char *) value1_l, sizeof(value1));   // recibe la operació
    if (err == -1) {
        printf("Error en recepcion de value2\n");
        close(sc);
        return -1;
    }

    err = sendMessage ( sc, (char *) &N_value2_l, sizeof(int));   // recibe la operació
    if (err == -1) {
        printf("Error en recepcion de value3\n");
        close(sc);
        return -1;
    }


    for ( int i = 0; i < N_value2; i++){
        err = sendMessage ( sc, (char *)&V_value2_l[i], sizeof(double));   // recibe la operació
        if (err == -1) {
            printf("Error en recepcion de value4\n");
            close(sc);
            return -1;
        }

    }


    err = sendMessage ( sc, (char *) &V_value3_x, sizeof(int));   // recibe la operació
    if (err == -1) {
        printf("Error en recepcion de value5\n");
        close(sc);
        return -1;
    }


    err = sendMessage ( sc, (char *) &V_value3_y, sizeof(int));   // recibe la operació
    if (err == -1) {
        printf("Error en recepcion de value6\n");
        close(sc);
        return -1;
    }
    printf("petición enviada");
    return 1;


}


int recivir_respuesta(int sc ){
    int key_l;
    char value1_l[256];
    int N_value2_l;
    double V_value2_l;
    int value3_l;
    int status_l; // Éxito o fracaso

    printf("esperando respuesta\n");

    int err = recvMessage(sc, (char *)&status_l, sizeof(int));  // envía el resultado
    if (err == -1) {
        printf("Error en recibir 1\n");
        close(sc);
        return -1;
    }

    status= ntohl(status_l);
    if ( status == -1){
        printf("%d \n", status);
        return 1;
    }


    err = recvMessage(sc, (char *)&key_l, sizeof(int));  // envía el resultado
    if (err == -1) {
        printf("Error en envio2\n");
        close(sc);
        return -1;
    }
    key = ntohl(key_l);

    err = recvMessage(sc, value1_l, sizeof(value1_l));  // envía el resultado
    if (err == -1) {
        printf("Error en envio3\n");
        close(sc);
        return -1;
    }
    strcpy(value1, value1_l);

    err = recvMessage(sc, (char *)&N_value2_l, sizeof(int));  // envía el resultado
    if (err == -1) {
        printf("Error en envio4\n");
        close(sc);
        return -1;
    }
    N_value2 = ntohl(N_value2_l);

    for ( int i = 0; i < N_value2 ; i++){
        err = recvMessage(sc, (char *)&V_value2_l, sizeof(double));  // envía el resultado
        if (err == -1) {
            printf("Error en envio5\n");
            close(sc);
            return -1;
        }
        V_value2[i] = ntohl(V_value2_l);
    }


    err = recvMessage(sc, (char *)&value3_l, sizeof(int));  // envía el resultado
    if (err == -1) {
        printf("Error en envio6\n");
        close(sc);
        return -1;
    }
    value3.x = ntohl(value3_l);

    err = recvMessage(sc, (char *)&value3_l, sizeof(int));  // envía el resultado
    if (err == -1) {
        printf("Error en envio7\n");
        close(sc);
        return -1;
        }
    value3.y = ntohl(value3_l);


    close(sc);
    return 1;

}

int destroy() {
    printf("Mensaje del proxy: Enviando una petición destroy\n");
    int sc = make_conection();

    if (sc == -1){
        return -1;
    }
    op = OP_DESTROY;
    if (enviar_peticion(sc ) == -1){
        close(sc);
        exit(-1);
    }
    printf("petición mandada \n");
    if ( recivir_respuesta(sc) == -1 ){
        close(sc);
        exit(-1);
    }
    printf("respueta: %d\n", status);

    return status;
}

int set_value(int key_i, char *value1_i, int N_value2_i, double *V_value2_i, struct Coord value3_i) {
    printf("Mensaje del proxy: Enviando una petición set_value\n");


    op = OP_SET;
    key = key_i;
    strcpy(value1, value1_i);
    N_value2 = N_value2_i;
    memcpy(V_value2, V_value2_i, sizeof(double) * N_value2);
    value3 = value3_i;

    int sc = make_conection();
    enviar_peticion( sc );
    recivir_respuesta( sc);

    close(sc);

    return status;
}

int get_value(int key_i, char *value1_i, int *N_value2_i, double *V_value2_i, struct Coord *value3_i) {
    printf("Mensaje del proxy: Enviando una petición get_value\n");



    op = OP_SET;
    key = key_i;
    strcpy(value1, value1_i);
    N_value2 = N_value2_i;
    memcpy(V_value2, V_value2_i, sizeof(double) * N_value2);
    value3 = *value3_i;

    int sc = make_conection();
    if (sc == -1){
        return -1;
    }
    enviar_peticion(sc );
    recivir_respuesta(sc);

    close(sc);

    if (status == 0){
        strcpy(value1_i, value1);
        *N_value2_i = N_value2;
        memcpy(V_value2_i, V_value2, sizeof(double) * (N_value2));
        *value3_i = value3;
    }

    return status;
}

int modify_value(int key_i, char *value1_i, int N_value2_i, double *V_value2_i, struct Coord value3_i) {
    printf("Mensaje del proxy: Enviando una petición modify_value\n");


    op = OP_MODIFY;
    key = key_i;
    strcpy(value1, value1_i);
    N_value2 = N_value2_i;
    memcpy(V_value2, V_value2_i, sizeof(double) * N_value2);
    value3 = value3_i;


    int sc = make_conection();
    enviar_peticion(sc );
    recivir_respuesta(sc);

    close(sc);

    return status;
}

int delete_key(int key_i) {
    printf("Mensaje del proxy: Enviando una petición delete_key\n");

    op = OP_DELETE;
    key = key_i;


    int sc = make_conection();
    enviar_peticion(sc );
    recivir_respuesta(sc);

    close(sc);

    return status;
}

int exist(int key_i) {
    printf("Mensaje del proxy: Enviando una petición exist\n");

    op = OP_EXIST;
    key = key_i;

    int sc = make_conection();
    enviar_peticion(sc );
    recivir_respuesta(sc);

    close(sc);

    return status;
}
