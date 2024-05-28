#include "connection.h"

void broadcast_send_message(char* message, char* ip, int port){
    int udpSocket;
    struct sockaddr_in udpAddr;
    int broadcastPermission = 1;

    // Création du socket UDP
    if ((udpSocket = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Activation de l'option SO_BROADCAST
    if (setsockopt(udpSocket, SOL_SOCKET, SO_BROADCAST, &broadcastPermission, sizeof(broadcastPermission)) < 0) {
        perror("setsockopt for SO_BROADCAST failed");
        exit(EXIT_FAILURE);
    }

    // Configuration de l'adresse du serveur
    udpAddr.sin_family = AF_INET;
    udpAddr.sin_port = htons(port);
    udpAddr.sin_addr.s_addr = inet_addr(ip); // Adresse de diffusion pour envoyer à toutes les interfaces

    // Envoi du message de diffusion au serveur
    if (sendto(udpSocket, message, strlen(message), 0, (struct sockaddr *)&udpAddr, sizeof(udpAddr)) < 0) {
        perror("Send failed");
        close(udpSocket);
        exit(EXIT_FAILURE);
    }
    printf("Client sent: %s\n", message);

    close(udpSocket);
}
