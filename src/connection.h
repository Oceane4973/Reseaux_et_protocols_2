#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#ifndef CONNECTION_H
#define CONNECTION_H

#define BUFFER_SIZE 1024

typedef struct {
    int socket_id;
    struct sockaddr_in udpAddr;
} Connection;

void broadcast_send_message(char* message, char* ip, int port);
void send_yaml_file(const char *filename, int socket);
void receive_and_print_yaml(int socket);
void broadcast_send_file(char* filename, char* ip, int port);

#endif