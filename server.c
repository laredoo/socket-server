#include "common.h"

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/socket.h>
#include <sys/types.h>

int client_count = 0;
pthread_mutex_t count_mutex = PTHREAD_MUTEX_INITIALIZER; // Mutex para proteger o contador de clientes

void usage(int argc, char **argv) {
  printf("usage: %s <ipv4|ipv6> <server port>\n", argv[0]);
  printf("example: %s ipv4 51511\n", argv[0]);
  exit(EXIT_FAILURE);
}

struct client_data {
  int socket;
  struct sockaddr_storage storage;
  char movie_id;
};

void send_message(int movie_id, int message_id, int s,
                  struct sockaddr_storage cstorage, socklen_t caddrlen) {

  int line_id = (movie_id == 1) ? message_id : (movie_id - 1) * message_id + 5;

  FILE *arquivo;
  arquivo = fopen("../frases.txt", "r");
  if (arquivo == NULL) {
    printf("Erro ao abrir o arquivo.");
    logexit("File is null");
  }
  int counter = 0;
  char linha[100];

  while (fgets(linha, sizeof(linha), arquivo) != NULL) {
    if (counter == line_id)
      break;
    counter++;
  }

  sendto(s, linha, strlen(linha) + 1, 0, (struct sockaddr *)&cstorage,
         caddrlen);
}

void *client_thread(void *data) {

  struct client_data *cdata = (struct client_data *)data;
  socklen_t caddrlen = sizeof(cdata->storage);

  for (int i = 0; i < 5; i++) {
    send_message(atoi(&cdata->movie_id), i, cdata->socket, cdata->storage,
                 caddrlen);
    sleep(3);
  }

  printf("\n");

  pthread_mutex_lock(&count_mutex); // Trava o mutex
  client_count -= 1;
  pthread_mutex_unlock(&count_mutex); // Destrava o mutex

  close(cdata->socket);
  free(cdata);
  pthread_exit(EXIT_SUCCESS);
}

void *monitor_thread(void *arg) {
  while (1) {
    sleep(4);
    pthread_mutex_lock(&count_mutex); // Trava o mutex
    printf("Clientes conectados: %d\n", client_count);
    pthread_mutex_unlock(&count_mutex); // Destrava o mutex
  }
  pthread_exit(NULL);
}

int main(int argc, char **argv) {

  if (argc < 3) {
    usage(argc, argv);
  }

  struct sockaddr_storage
      storage; // Declara uma estrutura de armazenamento de endereço genérica.
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

  pthread_t monitor_tid;
  pthread_create(&monitor_tid, NULL, monitor_thread, NULL);

  while (1) {
    struct sockaddr_storage cstorage;

    struct sockaddr *caddr =
        (struct sockaddr *)(&cstorage); // Declara um ponteiro para a estrutura
                                        // de endereço genérica.
    socklen_t caddrlen =
        sizeof(cstorage); // Declara uma variável para armazenar o tamanho da
                          // estrutura de endereço do cliente.

    int client_socket;
    client_socket = socket(storage.ss_family, SOCK_DGRAM,
              0); // Cria um novo socket usando o tipo de família de protocolos e
                  // tipo de socket fornecidos.
    if (s == -1) {
      logexit("socket");
    }

    char USER_CHOICE[ANWSZ];
    char msg[BUFSZ];
    memset(USER_CHOICE, 0, ANWSZ);
    memset(msg, 0, BUFSZ);
    int bytes_recv = recvfrom(s, USER_CHOICE, ANWSZ, 0, caddr, &caddrlen);
    if (bytes_recv < 0) {
      logexit("recvfrom");
    } else {
      pthread_mutex_lock(&count_mutex); // Trava o mutex
      client_count += 1;
      pthread_mutex_unlock(&count_mutex); // Destrava o mutex
      char movie_id = USER_CHOICE[0];
      printf("Filme escolhido pelo cliente: %c\n", movie_id);
      struct client_data *cdata = malloc(sizeof(*cdata));
      if (!cdata) {
        logexit("malloc");
      }
      cdata->socket = client_socket;
      memcpy(&(cdata->storage), &cstorage, sizeof(cstorage));
      cdata->movie_id = movie_id;
      pthread_t tid;
      pthread_create(&tid, NULL, client_thread, cdata);
    }
  }
  exit(EXIT_SUCCESS);
}
