#include "connection.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <net/if.h>

void hello_world();
void send_tram();

int main() {
    //hello_world();
    send_tram();

    return 0;
}

void hello_world(){
    char* message = "Hello, World!";
    char* ip = "172.16.180.2";
    int port = 8080;

    send_message_on_standard_socket(message, strlen(message), ip, port);
}

void send_tram() {
    Tram *tram = (Tram *)malloc(sizeof(Tram));
    if (!tram) {
        perror("Failed to allocate memory for Tram");
        exit(EXIT_FAILURE);
    }

    tram->destination = "172.16.180.2";
    tram->origin = "127.0.0.1";
    tram->port = 8080;
    tram->message = "Hello";

    char* message = tram_to_buffer(tram);

    char *ip = "10.1.6.2";
    int port = 8520;

    // Envoyer le message sur le socket en utilisant la taille correcte du buffer
    send_message_on_standard_socket(message, strlen(message),ip, port);
}
