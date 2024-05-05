#include "common.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>

#define BUFSZ 1024
#define ANWSZ 3
#define REFUSE '0'
#define ACCEPT '1'

void usage(int argc, char **argv) {
  printf("usage: %s <server IP> <server port>\n", argv[0]);
  printf("example: %s 127.0.0.1 51511\n", argv[0]);
  exit(EXIT_FAILURE);
}

int main(int argc, char **argv) {
  if (argc < 3) {
    usage(argc, argv);
  }

  while(1) {

    char user_choice[ANWSZ];
    memset(user_choice, 0, ANWSZ);
    printf("Escolha uma opção>\n0 - Sair\n1 - Solicitar corrida\n");
    fgets(user_choice, sizeof(user_choice), stdin);
    
    if(user_choice[0] == REFUSE)
      break;

    if(user_choice[0] == ACCEPT) {
    
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
      if (0 != connect(s, addr, sizeof(storage))) { // conecta com o endereço passado
        logexit("connect");
      }

      char addrstr[BUFSZ];
      addrtostr(addr, addrstr, BUFSZ);
      printf("connected to %s\n", addrstr);

      char server_choice[ANWSZ];
      memset(server_choice, 0, ANWSZ);
      char server_message[BUFSZ];
      memset(server_message, 0, BUFSZ);

      while(1) {

        size_t bytes_recv = recv(s, server_choice, sizeof(server_choice), 0); // Consertar esse ponteiro para server choice

        if(server_choice[0] == REFUSE) {
          bytes_recv = recv(s, server_message, sizeof(server_message), 0); 
          printf("%s", server_message);
          close(s);
          break;
        }

        if(bytes_recv <= 0)
          logexit("Client received 0 bytes or less from server\n");
      }
    }
  }
}