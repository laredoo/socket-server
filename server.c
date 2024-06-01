#include "common.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

#include <sys/socket.h>
#include <sys/types.h>

void usage(int argc, char **argv) {
  printf("usage: %s <ipv4|ipv6> <server port>\n", argv[0]);
  printf("example: %s ipv4 51511\n", argv[0]);
  exit(EXIT_FAILURE);
}

struct client_data {
  int socket;
  struct sockaddr_storage storage;
};

void *client_thread(void *data) {

  struct client_data *cdata = (struct client_data *) data;
  struct sockaddr *caddr = (struct sockaddr *)(&cdata->storage); // Declara um ponteiro para a estrutura de endereço genérica.
  socklen_t caddrlen = sizeof(cdata->storage);

  char msg[BUFSZ];
  memset(msg, 0, BUFSZ);
  int bytes_recv = recvfrom(cdata->socket, msg, BUFSZ, 0, (struct sockaddr *)&caddr, &caddrlen);
  if(bytes_recv < 0) {
    logexit("recvfrom");
  } else {
    printf("Recebido do cliente: %s\n", msg);
    printf("Número de bytes recebidos: %d\n", bytes_recv);
    sendto(cdata->socket, "Olá do servidor", strlen("Olá do servidor"), 0, (struct sockaddr *)&cdata->storage, caddrlen);
  }

  close(cdata->socket);
  pthread_exit(EXIT_SUCCESS);
}

int main(int argc, char **argv) {

  if (argc < 3) {
    usage(argc, argv);
  }

  struct sockaddr_storage storage; // Declara uma estrutura de armazenamento de endereço genérica.
  if (0 != server_sockaddr_init(
               argv[1], argv[2],
               &storage)) { // Inicializa a estrutura de armazenamento de
                            // endereço com o endereço IP e a porta fornecidos.
    usage(argc, argv);
  }

  int s;
  s = socket(storage.ss_family, SOCK_DGRAM,
             0); // Cria um novo socket usando o tipo de família de protocolos e
                 // tipo de socket fornecidos.
  if (s == -1) {
    logexit("socket");
  }

  int enable = 1;
  if (0 !=
      setsockopt( // Configura a opção SO_REUSEADDR no socket.
          s, SOL_SOCKET, SO_REUSEADDR, &enable,
          sizeof(int))) {  // possibilita reutilizar o mesmo endereço de socket
    logexit("setsockopt"); // utilize enable = 0 para desabilitar essa opção
  }

  struct sockaddr *addr =
      (struct sockaddr *)(&storage); // Faz a ligação do socket a um endereço e
                                     // porta especificados.
  if (0 != bind(s, addr, sizeof(storage))) {
    logexit("bind");
  }

  char addrstr[BUFSZ];
  addrtostr(addr, addrstr,
            BUFSZ); // Converte o endereço do servidor em uma string legível por
                    // humanos e imprime uma mensagem indicando que o servidor
                    // está aguardando conexões.
  printf("UDP server está escutando\n");

  while(1) {

    struct sockaddr_storage cstorage;

    struct sockaddr *caddr =
        (struct sockaddr *)(&cstorage); // Declara um ponteiro para a estrutura
                                        // de endereço genérica.
    socklen_t caddrlen =
        sizeof(cstorage); // Declara uma variável para armazenar o tamanho da
                          // estrutura de endereço do cliente.

    char msg[BUFSZ];
    memset(msg, 0, BUFSZ);
    int bytes_recv = recvfrom(s, msg, BUFSZ, 0, caddr, &caddrlen);
    if(bytes_recv < 0) {
      logexit("recvfrom");
    }
    printf("Número de bytes recebidos: %d\n", bytes_recv);
    printf("Recebido do cliente: %s", msg);
    
    strcpy(msg, "Message received, Cench.\n");
    printf("Mensagem enviada: %s", msg);
    sendto(s, msg, strlen(msg)+1, 0, (struct sockaddr *)&cstorage, caddrlen);

    // pthread_t tid;
    // pthread_create(&tid, NULL, client_thread, cdata);
    // pthread_detach(tid);
    
  }
  close(s);
  exit(EXIT_SUCCESS);
}

