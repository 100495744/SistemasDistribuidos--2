#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mqueue.h>
#include "claves.h"

typedef struct {
    int op; // Operación a realizar
    int key;
    char value1[256];
    int N_value2;
    double V_value2[32];
    struct Coord value3;
    char q_name[256]; // Nombre de la cola de mensajes
} peticion;

typedef struct {
    int key;
    char value1[256];
    int N_value2;
    double V_value2[32];
    struct Coord value3;
    int status; // Éxito o fracaso
} respuesta;

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

int abrir_colas(mqd_t *qs, mqd_t *qr, char *qr_name) {
    inicializar_attr();
    
    // Generar nombre de cola de respuesta único
    sprintf(qr_name, "%s%d", "/CLIENTE_", getpid());
    
    // Abrir cola de servidor 
    *qs = mq_open("/SERVIDOR", O_CREAT | O_WRONLY, 0700, &attr_peticion);
    if (*qs == -1) {
        perror("Error al abrir la cola de mensajes (servidor)");
        return -1;
    }
    
    // Abrir cola de respuesta
    *qr = mq_open(qr_name, O_CREAT | O_RDONLY, 0700, &attr_respuesta);
    if (*qr == -1) {
        perror("Error al abrir la cola de mensajes (cliente)");
        mq_close(*qs);
        return -1;
    }
    
    return 0;
}

// Función de limpieza de colas
void limpiar_colas(mqd_t qs, mqd_t qr, char *qr_name) {

    if (mq_close(qs) == -1) {
        perror("Error al cerrar la cola del servidor");
    }

    if (mq_close(qr) == -1) {
        perror("Error al cerrar la cola del cliente");
    }

    if (mq_unlink(qr_name) == -1) {
        perror("Error al desvincular la cola del cliente");
    }
}

int destroy() {
    printf("Mensaje del proxy: Enviando una petición destroy\n");
    peticion peticion;
    respuesta respuesta;
    char qr_name[1024];
    int prio;
    mqd_t qs, qr;

    if (abrir_colas(&qs, &qr, qr_name) == -1) {
        return -1;
    }

    peticion.op = OP_DESTROY;
    strcpy(peticion.q_name, qr_name);

    mq_send(qs, (char *)&peticion, sizeof(peticion), 0);
    mq_receive(qr, (char *)&respuesta, sizeof(respuesta), &prio);
    
    limpiar_colas(qs, qr, qr_name);

    return respuesta.status;
}

int set_value(int key, char *value1, int N_value2, double *V_value2, struct Coord value3) {
    printf("Mensaje del proxy: Enviando una petición set_value\n");
    peticion peticion;
    respuesta respuesta;
    char qr_name[1024];
    int prio;
    mqd_t qs, qr;

    if (abrir_colas(&qs, &qr, qr_name) == -1) {
        return -1;
    }
    
    peticion.op = OP_SET;
    peticion.key = key;
    strcpy(peticion.value1, value1);
    peticion.N_value2 = N_value2;
    memcpy(peticion.V_value2, V_value2, sizeof(double) * N_value2);
    peticion.value3 = value3;
    strcpy(peticion.q_name, qr_name);

    mq_send(qs, (char *)&peticion, sizeof(peticion), 0);
    ssize_t bytes_read = mq_receive(qr, (char *)&respuesta, sizeof(respuesta), 0);
    if (bytes_read == -1) {
        perror("Error en mq_receive");
        return -1;
    }

    limpiar_colas(qs, qr, qr_name);

    return respuesta.status;
}

int get_value(int key, char *value1, int *N_value2, double *V_value2, struct Coord *value3) {
    printf("Mensaje del proxy: Enviando una petición get_value\n");
    peticion peticion;
    respuesta respuesta;
    char qr_name[1024];
    int prio;
    mqd_t qs, qr;

    if (abrir_colas(&qs, &qr, qr_name) == -1) {
        return -1;
    }

    peticion.op = OP_GET;
    peticion.key = key;
    strcpy(peticion.q_name, qr_name);
    mq_send(qs, (char *)&peticion, sizeof(peticion), 0);
    ssize_t bytes_read = mq_receive(qr, (char *)&respuesta, sizeof(respuesta), 0);
    if (bytes_read == -1) {
        perror("Error en mq_receive");
        return -1;
    }
    if (respuesta.status == 0){
        strcpy(value1, respuesta.value1);
        *N_value2 = respuesta.N_value2;
        memcpy(V_value2, respuesta.V_value2, sizeof(double) * (*N_value2));
        *value3 = respuesta.value3;
    }

    //DEBUG -> ELIMINAR
    /*printf("Soy el proxy y respuesta.value1 = %s\n", respuesta.value1);
    printf("Soy el proxy y respuesta.N_value2 = %d\n", respuesta.N_value2);
        for (int i = 0; i < respuesta.N_value2; i++){
            printf("Soy el proxy y respuesta.V_value2[%d] = %f\n", i, respuesta.V_value2[i]);
        }
    printf("Soy el proxy yrespuesta.value3 = (%d, %d)\n", respuesta.value3.x, respuesta.value3.y);*/

    limpiar_colas(qs, qr, qr_name);

    return respuesta.status;
}

int modify_value(int key, char *value1, int N_value2, double *V_value2, struct Coord value3) {
    printf("Mensaje del proxy: Enviando una petición modify_value\n");
    peticion peticion;
    respuesta respuesta;
    char qr_name[1024];
    int prio;
    mqd_t qs, qr;

    if (abrir_colas(&qs, &qr, qr_name) == -1) {
        return -1;
    }

    peticion.op = OP_MODIFY;
    peticion.key = key;
    strcpy(peticion.value1, value1);
    peticion.N_value2 = N_value2;
    memcpy(peticion.V_value2, V_value2, sizeof(double) * N_value2);
    peticion.value3 = value3;
    strcpy(peticion.q_name, qr_name);

    mq_send(qs, (char *)&peticion, sizeof(peticion), 0);
    mq_receive(qr, (char *)&respuesta, sizeof(respuesta), &prio);
    
    limpiar_colas(qs, qr, qr_name);

    return respuesta.status;
}

int delete_key(int key) {
    printf("Mensaje del proxy: Enviando una petición delete_key\n");
    peticion peticion;
    respuesta respuesta;
    char qr_name[1024];
    int prio;
    mqd_t qs, qr;

    if (abrir_colas(&qs, &qr, qr_name) == -1) {
        return -1;
    }

    peticion.op = OP_DELETE;
    peticion.key = key;
    strcpy(peticion.q_name, qr_name);

    mq_send(qs, (char *)&peticion, sizeof(peticion), 0);
    mq_receive(qr, (char *)&respuesta, sizeof(respuesta), &prio);
    
    limpiar_colas(qs, qr, qr_name);

    return respuesta.status;
}

int exist(int key) {
    printf("Mensaje del proxy: Enviando una petición exist\n");
    peticion peticion;
    respuesta respuesta;
    char qr_name[1024];
    int prio;
    mqd_t qs, qr;

    if (abrir_colas(&qs, &qr, qr_name) == -1) {
        return -1;
    }

    peticion.op = OP_EXIST;
    peticion.key = key;
    strcpy(peticion.q_name, qr_name);

    mq_send(qs, (char *)&peticion, sizeof(peticion), 0);
    mq_receive(qr, (char *)&respuesta, sizeof(respuesta), &prio);
    
    limpiar_colas(qs, qr, qr_name);

    return respuesta.status;
}
