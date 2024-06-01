#include "common.h"

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/socket.h>
#include <sys/types.h>

int client_count = 0; // Contador global para rastrear o número de clientes conectados
pthread_mutex_t count_mutex = PTHREAD_MUTEX_INITIALIZER; // Mutex para proteger o contador de clientes

void usage(int argc, char **argv) {
  // Exibe a forma correta de usar o programa e exemplos
  printf("usage: %s <ipv4|ipv6> <server port>\n", argv[0]);
  printf("example: %s ipv4 51511\n", argv[0]);
  exit(EXIT_FAILURE);
}

struct client_data {
  // Estrutura para armazenar os dados do cliente
  int socket; // Socket do cliente
  struct sockaddr_storage storage; // Endereço do cliente
  char movie_id; // ID do filme escolhido pelo cliente
};

void send_message(int movie_id, int message_id, int s,
                  struct sockaddr_storage cstorage, socklen_t caddrlen) {

  // Calcula a linha da mensagem a ser enviada com base no movie_id e message_id               
  int line_id = (movie_id == 1) ? message_id : (movie_id - 1) * message_id + 5;

  FILE *arquivo;
  arquivo = fopen("../frases.txt", "r"); // Abre o arquivo de frases
  if (arquivo == NULL) {
    printf("Erro ao abrir o arquivo.");
    logexit("File is null");
  }
  int counter = 0;
  char linha[100];

  // Lê a linha correta do arquivo
  while (fgets(linha, sizeof(linha), arquivo) != NULL) {
    if (counter == line_id)
      break;
    counter++;
  }

  // Envia a linha lida para o cliente
  sendto(s, linha, strlen(linha) + 1, 0, (struct sockaddr *)&cstorage,
         caddrlen);
}

void *client_thread(void *data) {
  // Thread que lida com a comunicação com um cliente

  struct client_data *cdata = (struct client_data *)data;
  socklen_t caddrlen = sizeof(cdata->storage);

  // Envia 5 mensagens para o cliente, uma a cada 3 segundos
  for (int i = 0; i < 5; i++) {
    send_message(atoi(&cdata->movie_id), i, cdata->socket, cdata->storage,
                 caddrlen);
    sleep(3);
  }

  printf("\n");

  pthread_mutex_lock(&count_mutex); // Trava o mutex antes de modificar o contador
  client_count -= 1; // Decrementa o contador de clientes
  pthread_mutex_unlock(&count_mutex); // Destrava o mutex

  close(cdata->socket); // Fecha o socket do cliente
  free(cdata); // Libera a memória alocada para os dados do cliente
  pthread_exit(EXIT_SUCCESS);
}

void *monitor_thread(void *arg) {
  // Thread que monitora e imprime a quantidade de clientes conectados a cada 4 segundos
  while (1) {
    sleep(4); // Aguarda 4 segundos
    pthread_mutex_lock(&count_mutex); // Trava o mutex
    printf("Clientes conectados: %d\n", client_count); // Imprime a quantidade de clientes conectados
    pthread_mutex_unlock(&count_mutex); // Destrava o mutex
  }
  pthread_exit(NULL);
}

int main(int argc, char **argv) {

  if (argc < 3) {
    usage(argc, argv); // Verifica se os argumentos são suficientes e, se não, exibe a forma correta de usar o programa
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
  pthread_create(&monitor_tid, NULL, monitor_thread, NULL); // Cria a thread de monitoramento

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
    memset(USER_CHOICE, 0, ANWSZ); // Limpa o buffer USER_CHOICE
    memset(msg, 0, BUFSZ); // Limpa o buffer msg
    int bytes_recv = recvfrom(s, USER_CHOICE, ANWSZ, 0, caddr, &caddrlen); // Recebe a escolha do cliente
    if (bytes_recv < 0) {
      logexit("recvfrom");
    } else {
      pthread_mutex_lock(&count_mutex); // Trava o mutex
      client_count += 1;
      pthread_mutex_unlock(&count_mutex); // Destrava o mutex
      char movie_id = USER_CHOICE[0]; // Obtém o ID do filme escolhido pelo cliente
      printf("Filme escolhido pelo cliente: %c\n", movie_id);
      struct client_data *cdata = malloc(sizeof(*cdata)); // Aloca memória para os dados do cliente
      if (!cdata) {
        logexit("malloc");
      }
      cdata->socket = client_socket;
      memcpy(&(cdata->storage), &cstorage, sizeof(cstorage)); // Copia o endereço do cliente
      cdata->movie_id = movie_id;
      pthread_t tid;
      pthread_create(&tid, NULL, client_thread, cdata); // Cria uma nova thread para lidar com o cliente
    }
  }
  exit(EXIT_SUCCESS);
}
