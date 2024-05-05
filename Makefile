all: 
	gcc -Wall -c common.c
	gcc -Wall client.c common.o -o client -lm
	gcc -Wall server.c common.o -o server -lm
	gcc -Wall -lpthread multi-thread/server-mt.c common.o -o multi-thread/server-mt -lm

clean:
	@rm -f common.o client.o client server server-mt