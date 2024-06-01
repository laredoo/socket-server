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

    char INPUT[ANWSZ];
    memset(INPUT, 0, ANWSZ);
    printf("Escolha uma opção>\n0 - Sair\n1 - Senhor dos aneis\n2 - O poderoso "
           "Chefão\n3 - Clube da Luta\n");
    fgets(INPUT, sizeof(INPUT), stdin);
    char user_choice = INPUT[0];

    if(user_choice != REFUSE && user_choice != SENHOR_DOS_ANEIS && user_choice != PODEROSO_CHEFAO && user_choice != CLUBE_DA_LUTA) {
      printf("Por favor escolha um filme válido.\n");
      exit(EXIT_FAILURE);
    }

    if (user_choice == REFUSE)
      break;
    else {
      struct sockaddr_storage storage; // Declara uma estrutura de armazenamento
                                       // de endereço genérica.
      // Se ocorrer um erro na conversão, chama a função de uso para mostrar
      // como usar o programa e sai. Converte os argumentos passados (endereço
      // IP e porta) em uma estrutura de endereço.
      if (0 != addrparse(argv[2], argv[3], &storage)) {
        usage(argc, argv);
      }

      // Cria um novo socket usando o tipo de família de protocolos e tipo de
      // socket fornecidos.
      int s;
      s = socket(storage.ss_family, SOCK_DGRAM, 0);
      if (s == -1) {
        logexit("socket");
      }

      // Converte a estrutura de armazenamento de endereço em uma estrutura de
      // endereço genérica.
      struct sockaddr *addr = (struct sockaddr *)(&storage);

      // Converte o endereço do servidor em uma string legível por humanos e
      // imprime uma mensagem indicando que a conexão foi estabelecida.
      char addrstr[BUFSZ];
      addrtostr(addr, addrstr, BUFSZ);
      // printf("connected to %s\n", addrstr);

      while (1) {
        // Converte a estrutura de armazenamento de endereço em uma estrutura de
        // endereço genérica.
        struct sockaddr *caddr = (struct sockaddr *)(&storage);

        // Declara um buffer para uso geral.
        char general_buffer[BUFSZ];
        memset(general_buffer, 0, BUFSZ);

        printf("\nFilme Escolhido: %s\n", INPUT);

        socklen_t addrlen = sizeof(storage);

        // Envia a escolha do usuário para o servidor
        sendto(s, INPUT, strlen(INPUT) + 1, 0,
               (struct sockaddr *)&storage, addrlen);
        memset(INPUT, 0, ANWSZ);
        for(int i = 0; i < 5; i++) {
          // Recebe a resposta do servidor
          int bytes_recv = recvfrom(s, general_buffer, BUFSZ, 0, caddr, &addrlen);
          if (bytes_recv < 0) {
            logexit("recvfrom");
          } else {
            printf("%s", general_buffer); // Imprime a resposta recebida do servidor
          }
          memset(general_buffer, 0, BUFSZ); // Limpa o buffer general_buffer
          printf("\n");
        }

        printf("\n");
        close(s); // Fecha o socket
        break;
      }
    }
  }

  printf("<Encerrar Programa>\n");
  exit(EXIT_SUCCESS);
}
