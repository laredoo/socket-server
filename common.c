#include "common.h"

#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <arpa/inet.h>

# define M_PI 3.14159265358979323846

void logexit(const char *msg) {
  perror(msg);
  exit(EXIT_FAILURE);
}

// Função para analisar e converter um endereço IP e uma porta para uma estrutura sockaddr_storage.
int addrparse(const char *addrstr, const char *portstr,
              struct sockaddr_storage *storage) {

  if (addrstr == NULL || portstr == NULL) { // Verifica se os argumentos de entrada são válidos.
    return -1;
  }

  // Converte a string da porta para um número inteiro.
  uint16_t port = (uint16_t)atoi(portstr); // unsigned short

  // Verifica se a porta é 0.
  if (port == 0) {
    return -1;
  }

  // Converte a porta para a ordem de bytes da rede.
  port = htons(port); // host to network short

  // Declara uma estrutura para armazenar um endereço IPv4.
  struct in_addr inaddr4; // 32-bit IP address

  // Tenta converter o endereço IPv4 fornecido para a estrutura in_addr.
  if (inet_pton(AF_INET, addrstr, &inaddr4)) {
    // Se a conversão for bem-sucedida, preenche a estrutura sockaddr_in com os dados.
    struct sockaddr_in *addr4 = (struct sockaddr_in *)storage; 
    addr4->sin_family = AF_INET; // Define a família de endereços como IPv4.
    addr4->sin_port = port; // Define a porta.
    addr4->sin_addr = inaddr4; // Define o endereço IP.
    return 0;
  }

  // Declara uma estrutura para armazenar um endereço IPv6.
  struct in6_addr inaddr6; // 128-bit IP address
  // Se a conversão for bem-sucedida, preenche a estrutura sockaddr_in6 com os dados.
  if (inet_pton(AF_INET6, addrstr, &inaddr6)) {
    struct sockaddr_in6 *addr6 = (struct sockaddr_in6 *)storage; 
    addr6->sin6_family = AF_INET6; // Define a família de endereços como IPv6.
    addr6->sin6_port = port;// Define a porta.
    memcpy(&(addr6->sin6_addr), &inaddr6, sizeof(inaddr6)); // Copia o endereço IPv6.
    return 0;
  }

  return -1;
}

/* Convert socket byte address to human friendly readable format  */
// Função para converter o endereço de byte do soquete para um formato legível por humanos.
void addrtostr(const struct sockaddr *addr, char *str, size_t strsize) {

  int version; // Versão do endereço IP (4 ou 6).
  char addrstr[INET6_ADDRSTRLEN + 1] = ""; // String para armazenar o endereço IP.
  uint16_t port; // Número da porta.

  // Verifica a família do endereço.
  if (addr->sa_family == AF_INET) { // Se for IPv4.
    version = 4;
    struct sockaddr_in *addr4 = (struct sockaddr_in *)addr; // Converte para sockaddr_in.

    // Converte o endereço IPv4 para uma string legível.
    if (!inet_ntop(AF_INET, &(addr4->sin_addr), addrstr,
                   INET6_ADDRSTRLEN + 1)) {
      logexit("ntop"); // Registra um erro se a conversão falhar.
    }

    // Converte a porta da rede para a ordem de bytes do host.
    port = ntohs(addr4->sin_port); // network to host short

  } else if (addr->sa_family == AF_INET6) { // Se for IPv6.
    version = 6;
    struct sockaddr_in6 *addr6 = (struct sockaddr_in6 *)addr; // Converte para sockaddr_in6.

    // Converte o endereço IPv6 para uma string legível.
    if (!inet_ntop(AF_INET6, &(addr6->sin6_addr), addrstr,
                   INET6_ADDRSTRLEN + 1)) {
      logexit("ntop");
    }

    // Converte a porta da rede para a ordem de bytes do host.
    port = ntohs(addr6->sin6_port); // network to host short

  } else {
    logexit("unknown protocol family");
  }

  // Se o ponteiro `str` for válido.
  if (str) {
    snprintf(str, strsize, "IPv%d %s %hu", version, addrstr, port);
  }
}

// Inicializa o socket com as entradas: Endereço de rede e porta.
int server_sockaddr_init(const char *proto, const char *portstr,
                         struct sockaddr_storage *storage) {

  uint16_t port = (uint16_t)atoi(portstr); // unsigned short
  if (port == 0) {
    return -1;
  }

  // Converte a porta para a ordem de bytes da rede.
  port = htons(port); // host to network short

  memset(storage, 0, sizeof(*storage));
  // Verifica o protocolo fornecido.
  if (0 == strcmp(proto, "ipv4")) {
    struct sockaddr_in *addr4 = (struct sockaddr_in *)storage; // Converte para sockaddr_in.
    addr4->sin_family = AF_INET; // Define a família de endereços como IPv4.
    addr4->sin_addr.s_addr = INADDR_ANY; // Define o endereço IP como qualquer interface disponível.
    addr4->sin_port = port; // Define a porta.
    return 0;
  } else if (0 == strcmp(proto, "ipv6")) {
    struct sockaddr_in6 *addr6 = (struct sockaddr_in6 *)storage; // Converte para sockaddr_in6.
    addr6->sin6_family = AF_INET6; // Define a família de endereços como IPv6.
    addr6->sin6_addr = in6addr_any; // Define o endereço IP como qualquer interface disponível.
    addr6->sin6_port = port; // Define a porta.
    return 0;
  } else {
    return -1;
  }
}

double haversine_distance(double lat1, double lon1, double lat2, double lon2) {
  double dLat = (lat2 - lat1) * M_PI / 180.0;
  double dLon = (lon2 - lon1) * M_PI / 180.0;

  lat1 = (lat1)*M_PI / 180.0;
  lat2 = (lat2)*M_PI / 180.0;

  double a =
      pow(sin(dLat / 2), 2) + pow(sin(dLon / 2), 2) * cos(lat1) * cos(lat2);
  double rad = 6371;
  double c = 2 * asin(sqrt(a));
  return rad * c;
}