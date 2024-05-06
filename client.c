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
      s = socket(storage.ss_family, SOCK_STREAM, 0);
      if (s == -1) {
        logexit("socket");
      }

      // Converte a estrutura de armazenamento de endereço em uma estrutura de endereço genérica.
      struct sockaddr *addr = (struct sockaddr *)(&storage);

      // Estabelece uma conexão com o endereço especificado.
      if (0 !=
          connect(s, addr, sizeof(storage))) { // conecta com o endereço passado
        logexit("connect");
      }

      // Converte o endereço do servidor em uma string legível por humanos e imprime uma mensagem indicando que a conexão foi estabelecida.
      char addrstr[BUFSZ];
      addrtostr(addr, addrstr, BUFSZ);
      printf("connected to %s\n", addrstr);

      // Declara um buffer para armazenar a escolha do servidor.
      char server_choice[ANWSZ];
      memset(server_choice, 0, ANWSZ);

      // Declara um buffer para uso geral.
      char general_buffer[BUFSZ];
      memset(general_buffer, 0, BUFSZ);

      while (1) {
        memset(general_buffer, 0, BUFSZ);

        // Recebe dados do servidor no buffer de escolha do servidor.
        // O tamanho do buffer é definido para ser o tamanho máximo menos 1 para deixar espaço para o terminador nulo.
        // Este terminador nulo é importante para garantir que a string seja corretamente terminada.
        size_t bytes_recv = recv(s, server_choice, sizeof(server_choice)-1,0); // Consertar esse ponteiro para server choice

        if (server_choice[0] == REFUSE) {
          // Se o servidor recusar a solicitação, recebe a mensagem de recusa no buffer geral.
          // Esta mensagem pode conter detalhes sobre o motivo da recusa.
          // Imprime a mensagem de recusa.
          bytes_recv = recv(s, general_buffer, sizeof(general_buffer), 0);
          printf("%s", general_buffer);
          // Fecha o socket.
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
          while(total_bytes_sent < strlen(coord_buf)) { // Envia as coordenadas do cliente para o servidor.
            // Envia partes do buffer de coordenadas enquanto houver dados a serem enviados.
            size_t num_bytes_sent = send(s, coord_buf + total_bytes_sent, strlen(coord_buf) - total_bytes_sent, 0); // Envio a mensagem "Não foi encontrado motorista"
            if (num_bytes_sent == -1) {
                logexit("send");
            }
            total_bytes_sent += num_bytes_sent;
          }

          /* Agora aguardo as respostas consecutivas do servidor */
          // Loop para receber respostas consecutivas do servidor.
          while(1) {
            memset(general_buffer, 0, BUFSZ); // Limpa o buffer geral.
            size_t bytes_recv = recv(s, general_buffer, BUFSZ-1, 0); // Recebe dados do servidor no buffer geral.
            // Verifica se nenhum byte foi recebido, o que indica que o motorista chegou.
            if(bytes_recv == 0) { // O motorista chegou
              printf("O motorista Chegou!\n");
              printf("<Encerrar Programa>\n");
              exit(EXIT_SUCCESS); // Sai do programa com um código de sucesso.
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