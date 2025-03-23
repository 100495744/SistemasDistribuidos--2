CC=gcc

all: servidor-mq libclases cliente1 cliente2

servidor-mq: servidor-sock.c
	gcc -o servidor-mq servidor-mq.c

libclases: proxy-sock.c
	gcc -shared -o libclaves.so -fPIC proxy-mq.c -lrt

cliente1: ./Test/app-cliente.c
	gcc -o ./Test/app-cliente ./Test/app-cliente.c -L. -lclaves -lrt

cliente2: ./Test/app-cliente2.c
	gcc -o ./Test/app-cliente2 ./Test/app-cliente2.c -L. -lclaves -lrt

