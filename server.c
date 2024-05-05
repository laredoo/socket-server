#include "common.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/socket.h>
#include <sys/types.h>

#define BUFSZ 1024
#define REFUSE 0
#define ACCEPT 1

void usage(int argc, char **argv) {
  printf("usage: %s <v4|v6> <server port>\n", argv[0]);
  printf("example: %s v4 51511\n", argv[0]);
  exit(EXIT_FAILURE);
}

int main(int argc, char **argv) {

  if (argc < 3) {
    usage(argc, argv);
  }

  struct sockaddr_storage storage;
  if (0 != server_sockaddr_init(argv[1], argv[2], &storage)) {
    usage(argc, argv);
  }

  int s;
  s = socket(storage.ss_family, SOCK_STREAM, 0);
  if (s == -1) {
    logexit("socket");
  }

  int enable = 1;
  if (0 !=
      setsockopt(
          s, SOL_SOCKET, SO_REUSEADDR, &enable,
          sizeof(int))) {  // possibilita reutilizar o mesmo endereço de socket
    logexit("setsockopt"); // utilize enable = 0 para desabilitar essa opção
  }

  struct sockaddr *addr = (struct sockaddr *)(&storage);
  if (0 != bind(s, addr, sizeof(storage))) {
    logexit("bind");
  }

  if (0 !=
      listen(s, 10)) { // 10 é o limite de conexões pendentes para tratamento
    logexit("listen");
  }

  char addrstr[BUFSZ];
  addrtostr(addr, addrstr, BUFSZ);
  // printf("bound to %s, waiting connections\n", addrstr);
  printf("Aguardando solicitação\n");

  while (1) {
    struct sockaddr_storage cstorage;
    struct sockaddr *caddr = (struct sockaddr *)(&cstorage);
    socklen_t caddrlen = sizeof(cstorage);

    int csock = accept(s, caddr, &caddrlen);
    if (csock == -1) {
      logexit("accept");
    }

    char caddrstr[BUFSZ];
    addrtostr(caddr, caddrstr, BUFSZ);
    printf("[LOG] connection from %s\n", caddrstr);

    printf("[LOG] Corrida disponível:\n0 - Recusar\n1 - Aceitar\n");
    char user_option[3];
    memset(user_option, 0, 3);
    fgets(user_option, 3, stdin);
    if (user_option[0] == '0')
    {
        close(csock);
        printf("Aguardando solicitação.\n");
    }

    size_t num_bytes_sent = send(csock, user_option, strlen(user_option) + 1, 0); // num bytes efetivamente transmitidos na rede
    if (num_bytes_sent != strlen(user_option) + 1) {
        logexit("send");
    }
  }
  exit(EXIT_SUCCESS);
}