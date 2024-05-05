#pragma once

#include <math.h>
#include <stdlib.h>

#include <arpa/inet.h>

#define BUFSZ 1024
#define ANWSZ 3
#define REFUSE '0'
#define ACCEPT '1'

typedef struct {
  double latitude;
  double longitude;
} Coordinate;

void logexit(const char *msg);

int addrparse(const char *addrstr, const char *portstr,
              struct sockaddr_storage *storage);

void addrtostr(const struct sockaddr *addr, char *str, size_t strsize);

int server_sockaddr_init(const char *proto, const char *portstr,
                         struct sockaddr_storage *storage);

double haversine_distance(double lat1, double lon1, double lat2, double lon2);
