#include "common.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/socket.h>
#include <sys/types.h>

void usage(int argc, char **argv) {
  printf("usage: %s <ipv4|ipv6> <server port>\n", argv[0]);
  printf("example: %s ipv4 51511\n", argv[0]);
  exit(EXIT_FAILURE);
}

int main(int argc, char **argv) {

  if (argc < 3) {
    usage(argc, argv);
  }

  struct sockaddr_storage storage; // Declara uma estrutura de armazenamento de endereço genérica.
  if (0 != server_sockaddr_init(argv[1], argv[2], &storage)) { // Inicializa a estrutura de armazenamento de endereço com o endereço IP e a porta fornecidos.
    usage(argc, argv);
  }

  int s;
  s = socket(storage.ss_family, SOCK_STREAM, 0); // Cria um novo socket usando o tipo de família de protocolos e tipo de socket fornecidos.
  if (s == -1) {
    logexit("socket");
  }

  int enable = 1;
  if (0 !=
      setsockopt(   // Configura a opção SO_REUSEADDR no socket.
          s, SOL_SOCKET, SO_REUSEADDR, &enable,
          sizeof(int))) {  // possibilita reutilizar o mesmo endereço de socket
    logexit("setsockopt"); // utilize enable = 0 para desabilitar essa opção
  }

  struct sockaddr *addr = (struct sockaddr *)(&storage); // Faz a ligação do socket a um endereço e porta especificados.
  if (0 != bind(s, addr, sizeof(storage))) {
    logexit("bind");
  }

  // Coloca o socket em um estado passivo, pronto para aceitar conexões de entrada.
  if (0 !=
      listen(s, 10)) { // O segundo argumento, 10, é o número máximo de conexões pendentes que podem ser enfileiradas para tratamento.
    logexit("listen");
  }

  char addrstr[BUFSZ]; 
  addrtostr(addr, addrstr, BUFSZ); // Converte o endereço do servidor em uma string legível por humanos e imprime uma mensagem indicando que o servidor está aguardando conexões.
  printf("Aguardando solicitação\n");

  while (1) {
    struct sockaddr_storage cstorage; // Declara uma estrutura de armazenamento de endereço genérica para o cliente.
    struct sockaddr *caddr = (struct sockaddr *)(&cstorage); // Declara um ponteiro para a estrutura de endereço genérica.
    socklen_t caddrlen = sizeof(cstorage); // Declara uma variável para armazenar o tamanho da estrutura de endereço do cliente.


    // Aceita uma conexão de entrada de um cliente no socket 's' (que foi configurado anteriormente para ouvir conexões)
    // e armazena o socket da conexão aceita em 'csock'.
    // O endereço do cliente é preenchido na estrutura 'caddr'.
    // Se ocorrer um erro ao aceitar a conexão, registra um erro e sai.
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
    size_t num_bytes_sent = 0; // Declara uma variável para controlar o número total de bytes enviados.
    while(total_bytes_sent < strlen(server_choice)) { // Enquanto o número total de bytes enviados for menor que o tamanho da escolha do servidor...
      // Envia a escolha do servidor para o cliente através do socket 'csock'.
      // Este loop garante que todos os bytes da escolha sejam enviados.
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
      size_t bytes_recv = recv(csock, client_coord_buf, sizeof(client_coord_buf),0); // Recebe as coordenadas do cliente conectado.
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
        size_t bytesWritten = send(csock, server_msg, strlen(server_msg) + 1, 0); // Envia a mensagem ao cliente informando a distância do motorista.
        if (bytesWritten != strlen(server_msg) + 1)
            logexit("Error at send");
        distance -= 400;
        memset(server_msg, 0, BUFSZ);
        sleep(2); // espera 2 segundos
      }

      printf("O motorista chegou!\n");
      close(csock);
      printf("Aguardando solicitação.\n");
    }
  }
  exit(EXIT_SUCCESS);
}