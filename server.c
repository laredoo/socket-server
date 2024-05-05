#include "common.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/socket.h>
#include <sys/types.h>

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
    char server_choice[ANWSZ];
    memset(server_choice, 0, ANWSZ);
    fgets(server_choice, sizeof(server_choice), stdin);
    size_t total_bytes_sent = 0;

    /* 
      Aqui envio a escolha do servidor:
        0 > REFUSE
        1 > ACCEPT
    */
    size_t num_bytes_sent = 0;
    while(total_bytes_sent < strlen(server_choice)) {
      num_bytes_sent = send(csock, server_choice, strlen(server_choice), 0); // Envio a escolha do "Uber"
      if (num_bytes_sent == -1) {
          logexit("send");
      }
      total_bytes_sent += num_bytes_sent;
    }

    char server_msg[BUFSZ];

    if (server_choice[0] == REFUSE)
    {
      sprintf(server_msg, "Não foi encontrado motorista\n");
      total_bytes_sent = 0;

      /* Aqui envio a mensagem de falha "Não foi encontrado motorista" */
      while(total_bytes_sent < strlen(server_msg)) {
        num_bytes_sent = send(csock, server_msg + total_bytes_sent, strlen(server_msg) - total_bytes_sent, 0); // Envio a mensagem "Não foi encontrado motorista"
        if (num_bytes_sent == -1) {
            logexit("send");
        }
        total_bytes_sent += num_bytes_sent;
      }
      close(csock); // Fecho a conexão com o socket do cliente
      printf("Aguardando solicitação.\n");
    }

    if (server_choice[0] == ACCEPT)
    {
      Coordinate server_coordinate = {-19.9227, -43.9451};

      /* Aqui recebo as coordenadas do cliente conectado */
      char client_coord_buf[ANWSZ];
      size_t bytes_recv = recv(csock, client_coord_buf, sizeof(client_coord_buf),0);
      if (bytes_recv <= 0){
        logexit("Client received 0 bytes or less from server\n");
      }
      
      Coordinate client_coordinate;
      sscanf(client_coord_buf, "%lf %lf", &client_coordinate.latitude, &client_coordinate.longitude);

      int distance = haversine_distance(
        client_coordinate.latitude, client_coordinate.longitude,
        server_coordinate.latitude, server_coordinate.longitude
      );
      
      while (distance > 0) {
        sprintf(server_msg, "Motorista a %dm\n", distance);

        /* Mandando ao cliente a distância do motorista */
        size_t bytesWritten = send(csock, server_msg, strlen(server_msg) + 1, 0);
        if (bytesWritten != strlen(server_msg) + 1)
            logexit("Error at send");
        distance -= 400;
        memset(server_msg, 0, BUFSZ);
        sleep(2); // espera 2 segundos
      }
    }

    printf("O motorista chegou!\n");
    close(csock);
    printf("Aguardando solicitação.\n");
  }
  exit(EXIT_SUCCESS);
}