#!/bin/bash

gcc -c lines.c
gcc -c claves.c

# Compilar la biblioteca dinámica
echo "Compilando la biblioteca dinámica..."
gcc -shared -o libclaves.so -fPIC proxy-sock.c lines.o -lrt
if [ $? -ne 0 ]; then
    echo "Error al compilar libclaves.so"
    exit 1
fi


gcc -c servidor-sock.c
echo "Compilando el servidor..."
# Compilar el servidor
gcc -o servidor-sock servidor-sock.o lines.o claves.o
if [ $? -ne 0 ]; then
    echo "Error al compilar servidor-mq"
    exit 1
fi

echo "Compilando el cliente..."
# Compilar el cliente
gcc -o app-cliente ./Test/app-cliente.c -L. -lclaves -lrt
if [ $? -ne 0 ]; then
    echo "Error al compilar app-cliente"
    exit 1
fi

echo "Compilando el cliente..."
# Compilar el cliente
gcc -o app-cliente2 ./Test/app-cliente2.c -L. -lclaves -lrt
if [ $? -ne 0 ]; then
    echo "Error al compilar app-cliente"
    exit 1
fi

# Establecer la variable de entorno
echo "Estableciendo LD_LIBRARY_PATH..."
export LD_LIBRARY_PATH=./

echo "Compilación completada con éxito."
