#include "connection.h"

Connection broadcast_connection(char* ip, int port){
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
    udpAddr.sin_addr.s_addr = inet_addr(ip); 
    
    Connection co;
    co.socket_id = udpSocket;
    co.udpAddr = udpAddr;
    return co;
}

void broadcast_send_message(char* message, char* ip, int port){
    Connection socket = broadcast_connection(ip, port);
    
    // Envoi du message de diffusion au serveur
    if (sendto(socket.socket_id, message, strlen(message), 0, (struct sockaddr *)&socket.udpAddr, sizeof(socket.udpAddr)) < 0) {
        perror("Send failed");
        close(socket.socket_id);
        exit(EXIT_FAILURE);
    }
    printf("Client sent: %s\n", message);

    close(socket.socket_id);
}

void broadcast_send_file(char* filename, char* ip, int port){
    Connection socket = broadcast_connection(ip, port);

    // Connect to server
    if (connect(socket.socket_id, (struct sockaddr *)&socket.udpAddr, sizeof(socket.udpAddr)) == -1) {
        perror("Failed to connect to server");
        exit(EXIT_FAILURE);
    }

    // Receive and print YAML file
    //receive_and_print_yaml(client_socket);
    send_yaml_file(filename, socket.socket_id);

    // Close socket
    close(socket.socket_id);
}

void send_yaml_file(const char *filename, int socket) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        perror("Failed to open file");
        exit(EXIT_FAILURE);
    }

    char buffer[BUFFER_SIZE];
    size_t bytes_read;
    while ((bytes_read = fread(buffer, 1, sizeof(buffer), file)) > 0) {
        if (send(socket, buffer, bytes_read, 0) == -1) {
            perror("Failed to send data");
            exit(EXIT_FAILURE);
        }
    }

    printf("Client sent: %s\n", buffer);

    fclose(file);
}

void receive_and_print_yaml(int socket) {
    char buffer[BUFFER_SIZE];
    ssize_t bytes_received;

    while ((bytes_received = recv(socket, buffer, sizeof(buffer), 0)) > 0) {
        fwrite(buffer, 1, bytes_received, stdout);
    }

    if (bytes_received == -1) {
        perror("Failed to receive data");
        exit(EXIT_FAILURE);
    }
}