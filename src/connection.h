#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#ifndef CONNECTION_H
#define CONNECTION_H

#define BUFFER_SIZE 1024

void broadcast_send_message(char* message, char* ip, int port);

#endif