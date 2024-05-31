#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include "tram.h"

#ifndef CONNECTION_H
#define CONNECTION_H

#define BUFFER_SIZE 1024

typedef struct {
    int socket_id;
    struct sockaddr_in addr;
} Connection;

void broadcast_send_message(char* message, char* ip, int port);
void send_message_on_standard_socket(char* message, size_t message_size, char* ip, int port);
Connection standard_connection(char* ip, int port);
Connection broadcast_connection(char* ip, int port);

#endif