all: 
	gcc -Wall -c common.c
	gcc -Wall client.c common.o -o bin/client -lm
	gcc -Wall server.c common.o -o bin/server -lm

clean:
	@rm -f common.o client.o bin/client bin/server
	