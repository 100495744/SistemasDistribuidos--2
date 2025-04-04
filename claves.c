#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include "claves.h"


char DirName[]= "DataBase";

int destroy(){
    //struct dirent *pDirent;
    DIR *Dir;

    Dir = opendir(DirName);

    if ( Dir != NULL){
       if (system("rm -r DataBase ") == -1){
            printf("Error al eliminar el directorio\n");
            closedir(Dir);
            return -1;
        }
    } else{
        printf("Error al abrir el directorio\n");
        return -1;
    }

    if (mkdir(DirName,S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) == -1){
        printf("Error al crear el directorio\n");
        return -1;
    }

    return 0;

}


int set_value(int key, char *value1, int N_value2, double *V_value2, struct Coord value3){
    //Este servicio inserta el elemento <key, value1, value2, value3>.
    /*El servicio devuelve 0 si se insertó con éxito y -1 en caso de error.
    Se considera error intentar insertar una clave que ya existe previamente, o que el 
    valor N_value2 esté fuera de rango (los valores tienen que estar comprendidos entre 1 y 32). 
    En este caso se devolverá -1 y no se insertará.*/

    DIR *Dir;
    Dir = opendir(DirName);

    char numstr[15];
    char filepath[256]; //number??
    // Hacer error checking de si hay tantos valores en el vector como se indica en N_value2? Mejor en la parte de cliente?

    if (Dir == NULL || N_value2 > 32 || N_value2 < 1 || V_value2 == NULL || value1 == NULL || ((value1 != NULL) && (strlen(value1) > 256))){

        return -1;
    }

    if (exist(key) == 1){
        printf("La clave ya existe\n");
        return -1;
    }
    
    sprintf(numstr,"%d.txt",key);
    sprintf(filepath, "%s/%s", DirName, numstr);
    FILE  *newfile = fopen(filepath, "w");

    if (newfile == NULL){   
            printf("Error creando el archivo\n");
            return -1; //Error creating the file
        }

    //Witing the values in the file    
    fprintf(newfile, "%s\n", value1);
    fprintf(newfile, "%d\n", N_value2);
    for (int i = 0; i < N_value2; i++)
        {
            fprintf(newfile, "%f\n", V_value2[i]);
        }
    fprintf(newfile, "%d\n", value3.x);
    fprintf(newfile, "%d\n", value3.y);
    fclose(newfile);

    return 0;
}

int get_value(int key, char *value1, int *N_value2, double *V_value2, struct Coord *value3){
    char filename[40];
    snprintf(filename, sizeof(filename), "DataBase/%d.txt", key);

    FILE *file = fopen(filename, "r");
    if (!file) {
        printf("No existe la clave\n");
        return -1;
    }
    if (fgets(value1, 256, file) == NULL) {
        fclose(file);
        return -1;
    }

    value1[strcspn(value1, "\n")] = 0; // Eliminar el salto de línea
    
    if (fscanf(file, "%d", N_value2) != 1){
        fclose(file);
        return -1;
    }
 
    for (int i = 0; i < *N_value2; i++) {
        if (fscanf(file, "%lf", &V_value2[i]) != 1) {
            fclose(file);
            return -1;
        }
    }

    if (fscanf(file, "%d" , &value3->x) != 1) {
        fclose(file);
        return -1;
    }
    if (fscanf(file, "%d" , &value3->y) != 1) {
        fclose(file);
        return -1;
    }

    fclose(file);
    return 0; // Éxito
}

int modify_value(int key, char *value1, int N_value2, double *V_value2, struct Coord value3){
    if (exist(key) == 0){
        printf("No existe la clave que se está intentando modificar\n");
        return -1;
    }
    if (delete_key(key) == -1){
        printf("Error al modificar la clave\n");
        return -1;
    }
    if (set_value(key, value1, N_value2, V_value2, value3) == -1){
        printf("Error al modificar la clave\n");
        return -1;
    }
    return 0;
}

int delete_key(int key){
    if (exist(key) == 0){
        printf("No existe la clave\n");
        return -1;
    }
    char filename[20];
    snprintf(filename, sizeof(filename), "DataBase/%d.txt", key);
    if (remove(filename) != 0) {
        perror("Error al eliminar el archivo");
        return -1;
    }
    return 0;
}

int exist(int key){
    DIR *Dir;
    struct dirent *dir;
    char numstr[15];

    Dir = opendir(DirName);
    sprintf(numstr,"%d",key);
    strcat(numstr, ".txt");

    //Si existe el directorio, buscamos el fichero llamado con el nombre de la clave
    if (Dir != NULL){
        while((dir = readdir(Dir)) != NULL){
            if (strcmp(dir->d_name,numstr) == 0){
                closedir(Dir);
                return 1;
            }
        }
    }
    closedir(Dir);
    //Si no existe el directorio, tampoco la clave
    return 0;
}