#include "common.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>

void usage(int argc, char **argv) {
  printf("usage: %s <ipv4|ipv6> <server IP> <server port>\n", argv[0]);
  printf("example: %s ipv4 127.0.0.1 51511\n", argv[0]);
  exit(EXIT_FAILURE);
}

int main(int argc, char **argv) {
  if (argc < 4) {
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

      struct sockaddr_storage storage; // Declara uma estrutura de armazenamento de endereço genérica.
      // Se ocorrer um erro na conversão, chama a função de uso para mostrar como usar o programa e sai.
      // Converte os argumentos passados (endereço IP e porta) em uma estrutura de endereço.
      if (0 != addrparse(argv[2], argv[3], &storage)) {
        usage(argc, argv);
      }

      // Cria um novo socket usando o tipo de família de protocolos e tipo de socket fornecidos.
      int s;
      s = socket(storage.ss_family, SOCK_DGRAM, 0);
      if (s == -1) {
        logexit("socket");
      }

      // Converte a estrutura de armazenamento de endereço em uma estrutura de endereço genérica.
      struct sockaddr *addr = (struct sockaddr *)(&storage);

      // Converte o endereço do servidor em uma string legível por humanos e imprime uma mensagem indicando que a conexão foi estabelecida.
      char addrstr[BUFSZ];
      addrtostr(addr, addrstr, BUFSZ);
      printf("connected to %s\n", addrstr);

      while (1) {
        // Converte a estrutura de armazenamento de endereço em uma estrutura de endereço genérica.
        struct sockaddr *caddr = (struct sockaddr *)(&storage);

        // Declara um buffer para uso geral.
        char general_buffer[BUFSZ];
        strcpy(general_buffer, "Hi from the client bruh. Cench here. Alright.\n");

        printf("Message sent: %s\n", general_buffer);

        socklen_t addrlen = sizeof(storage);

        sendto(s, general_buffer, strlen(general_buffer)+1, 0, (struct sockaddr *)&storage, addrlen);
        memset(general_buffer, 0, BUFSZ);

        int bytes_recv = recvfrom(s, general_buffer, BUFSZ, 0, caddr, &addrlen);
        if(bytes_recv < 0) {
          logexit("recvfrom");
        }
        else {
          printf("Número de bytes recebidos: %d\n", bytes_recv);
          printf("Recebido do cliente: %s", general_buffer);
        }

        close(s);
        
        break;
      }
    }
  }
  
  printf("<Encerrar Programa>\n");
  exit(EXIT_SUCCESS);
}