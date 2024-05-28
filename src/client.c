#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define BROADCAST_PORT 1900
#define BROADCAST_ADDR "192.1.2.255"
#define BUFFER_SIZE 1024

int main() {
    int udpSocket;
    struct sockaddr_in udpAddr;
    char buffer[BUFFER_SIZE];
    const char *message = "broadcast";
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
    udpAddr.sin_port = htons(BROADCAST_PORT);
    udpAddr.sin_addr.s_addr = inet_addr(BROADCAST_ADDR); // Adresse de diffusion pour envoyer à toutes les interfaces

    // Envoi du message de diffusion au serveur
    if (sendto(udpSocket, message, strlen(message), 0, (struct sockaddr *)&udpAddr, sizeof(udpAddr)) < 0) {
        perror("Send failed");
        close(udpSocket);
        exit(EXIT_FAILURE);
    }
    printf("Sent: %s\n", message);

    // Réception de la réponse du serveur (si nécessaire)
    // Notez que pour la diffusion, il n'y a généralement pas de réponse du serveur.
    // Si vous attendez une réponse, vous devrez peut-être gérer cela différemment.
    
    // Fermeture du socket
    close(udpSocket);

    return 0;
}
