#include "common.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>

void usage(int argc, char **argv) {
  printf("usage: %s <server IP> <server port>\n", argv[0]);
  printf("example: %s 127.0.0.1 51511\n", argv[0]);
  exit(EXIT_FAILURE);
}

int main(int argc, char **argv) {
  if (argc < 3) {
    usage(argc, argv);
  }

  while (1) {

    char user_choice[ANWSZ];
    memset(user_choice, 0, ANWSZ);
    printf("Escolha uma opção>\n0 - Sair\n1 - Solicitar corrida\n");
    fgets(user_choice, sizeof(user_choice), stdin);

    if (user_choice[0] == REFUSE)
      break;

    if (user_choice[0] == ACCEPT) {

      struct sockaddr_storage storage;
      if (0 != addrparse(argv[1], argv[2], &storage)) {
        usage(argc, argv);
      }

      int s;
      s = socket(storage.ss_family, SOCK_STREAM, 0);
      if (s == -1) {
        logexit("socket");
      }

      struct sockaddr *addr = (struct sockaddr *)(&storage);
      if (0 !=
          connect(s, addr, sizeof(storage))) { // conecta com o endereço passado
        logexit("connect");
      }

      char addrstr[BUFSZ];
      addrtostr(addr, addrstr, BUFSZ);
      printf("connected to %s\n", addrstr);

      char server_choice[ANWSZ];
      memset(server_choice, 0, ANWSZ);

      char general_buffer[BUFSZ];
      memset(general_buffer, 0, BUFSZ);

      while (1) {
        memset(general_buffer, 0, BUFSZ);
        size_t bytes_recv = recv(s, server_choice, sizeof(server_choice)-1,0); // Consertar esse ponteiro para server choice

        if (server_choice[0] == REFUSE) {
          bytes_recv = recv(s, general_buffer, sizeof(general_buffer), 0);
          printf("%s", general_buffer);
          close(s);
          break;
        }
        if (bytes_recv <= 0)
          logexit("Client received 0 bytes or less from server\n");

        if (server_choice[0] == ACCEPT) {
          Coordinate client_coordinate = {-19.88786, -43.99599};
          char coord_buf[BUFSZ];
          memset(coord_buf, 0, BUFSZ);
          sprintf(coord_buf, "%lf %lf", client_coordinate.latitude, client_coordinate.longitude);

          /* Aqui envio a mensagem ao servidor informando as coordenadas do meu client*/
          size_t total_bytes_sent = 0;
          while(total_bytes_sent < strlen(coord_buf)) {
            size_t num_bytes_sent = send(s, coord_buf + total_bytes_sent, strlen(coord_buf) - total_bytes_sent, 0); // Envio a mensagem "Não foi encontrado motorista"
            if (num_bytes_sent == -1) {
                logexit("send");
            }
            total_bytes_sent += num_bytes_sent;
          }

          /* Agora aguardo as respostas consecutivas do servidor */
          while(1) {
            memset(general_buffer, 0, BUFSZ);
            size_t bytes_recv = recv(s, general_buffer, BUFSZ-1, 0);
            if(bytes_recv == 0) { // O motorista chegou
              printf("O motorista Chegou!\n");
              printf("<Encerrar Programa>\n");
              exit(EXIT_SUCCESS);
            }
            printf("%s\n", general_buffer);
          }

          close(s);
        }
        break;
      }
    }
  }
  printf("<Encerrar Programa>\n");
  exit(EXIT_SUCCESS);
}