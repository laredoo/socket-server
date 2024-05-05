#include "common.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>

#define BUFSZ 1024

void usage(int argc, char **argv) {
  printf("usage: %s <server IP> <server port>\n", argv[0]);
  printf("example: %s 127.0.0.1 51511\n", argv[0]);
  exit(EXIT_FAILURE);
}


int main(int argc, char **argv) {
  if (argc < 3) {
    usage(argc, argv);
  }


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
  if (0 != connect(s, addr, sizeof(storage))) {
    logexit("connect");
  }

  char addrstr[BUFSZ];
  addrtostr(addr, addrstr, BUFSZ);

  printf("connected to %s\n", addrstr);

  char buf[BUFSZ];
  memset(buf, 0, BUFSZ);
  printf("mensagem> ");
  fgets(buf, BUFSZ - 1, stdin);
  size_t count = send(s, buf, strlen(buf) + 1,
                   0); // num bytes efetivamente transmitidos na rede
  if (count != strlen(buf) + 1) {
    logexit("send");
  }

  memset(buf, 0, BUFSZ);
  unsigned total = 0; // total de bytes recebidos até o momento
  while (1) {
    count = recv(s, buf + total, BUFSZ - total,
                 0); // recebe a resposta do sv e coloco total bytes para frente
    if (count == 0) {
      // Conexão terminada. Apenas recebemos 0 bytes do servidor se não tiver
      // mais conexão
      break;
    }
    total += count;
  }
  close(s);

  printf("received %u bytes\n", total);
  puts(buf);

  exit(EXIT_SUCCESS);
}