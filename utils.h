#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include "parson.h"

#define MAX_IP 16
#define BUFFLEN 4096
#define LINELEN 1000
#define SERVER_IP "63.32.125.183"
#define SERVER_PORT 8081
#define HTTP_PORT 80
#define PAYLOAD_TYPE "application/json"
#define USERNAME "eduard_stefan.pana"
#define PASSWORD "6cf76b1864c6"


// define-ul opreste executia programului daca asertiunea este adevarata
#define DIE(assertion, call_description)                                       \
  do {                                                                         \
    if (assertion) {                                                           \
      fprintf(stderr, "(%s, %d): ", __FILE__, __LINE__);                       \
      perror(call_description);                                                \
      exit(EXIT_FAILURE);                                                      \
    }                                                                          \
  } while (0)

#endif