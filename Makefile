all: 
	gcc -Wall -c common.c
	gcc -Wall client.c common.o -o client
	gcc -Wall server.c common.o -o server
	gcc -Wall -lpthread multi-thread/server-mt.c common.o -o multi-thread/server-mt