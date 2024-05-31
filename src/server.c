#include "server.h"
#include "connection.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>

void destroyServer(Server *server) {
    if (server->name) free(server->name);
    if (server->ip) free(server->ip);
    free(server);
}

void *start_server(void *arg) {
    Server *server = (Server *)arg;

    int new_socket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    char buffer[1024] = {0};
    char *confirmation_message = "Message received";

    Connection socketConnection = standard_connection(server->ip, server->port);
    socklen_t tcp_addr_len = sizeof(socketConnection.addr);

    if (bind(socketConnection.socket_id, (struct sockaddr *)&socketConnection.addr, tcp_addr_len) < 0) {
        perror("Server socket bind failed");
        pthread_exit(NULL);
    }

    if (listen(socketConnection.socket_id, 5) < 0) {
        perror("Server socket listen failed");
        pthread_exit(NULL);
    }

    while (1) {
        printf("%s         Server listening on %s:%d\n", server->name, server->ip, server->port);

        if ((new_socket = accept(socketConnection.socket_id, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
            perror("Server socket Accept");
            continue; 
        }

        int valread = read(new_socket, buffer, 1024);
        if (valread < 0) {
            perror("Server socket Read");
            close(new_socket);
            continue;
        }

        printf("%s         Received message: %s\n", server->name, buffer);

        send(new_socket, confirmation_message, strlen(confirmation_message), 0);
        printf("%s         Confirmation message sent\n", server->name);

        close(new_socket);
    }

    close(socketConnection.socket_id);
    pthread_exit(NULL);
}
