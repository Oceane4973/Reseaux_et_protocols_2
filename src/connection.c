#include "connection.h"

Connection broadcast_connection(char* ip, int port){
    int udpSocket;
    struct sockaddr_in udpAddr;
    int broadcastPermission = 1;

    if ((udpSocket = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    if (setsockopt(udpSocket, SOL_SOCKET, SO_BROADCAST, &broadcastPermission, sizeof(broadcastPermission)) < 0) {
        perror("setsockopt for SO_BROADCAST failed");
        exit(EXIT_FAILURE);
    }

    udpAddr.sin_family = AF_INET;
    udpAddr.sin_port = htons(port);
    udpAddr.sin_addr.s_addr = inet_addr(ip); 
    
    Connection co;
    co.socket_id = udpSocket;
    co.addr = udpAddr;
    return co;
}

Connection standard_connection(char* ip, int port) {
    int tcpSocket;
    struct sockaddr_in tcpAddr;

    if ((tcpSocket = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    memset(&tcpAddr, 0, sizeof(tcpAddr));
    tcpAddr.sin_family = AF_INET;
    tcpAddr.sin_addr.s_addr = inet_addr(ip);
    tcpAddr.sin_port = htons(port);
    
    Connection co;
    co.socket_id = tcpSocket;
    co.addr = tcpAddr;
    return co;
}

void send_message_on_standard_socket(char* message, size_t message_size, char* ip, int port) {
    Connection socket = standard_connection(ip, port);

    if (connect(socket.socket_id, (struct sockaddr *)&socket.addr, sizeof(socket.addr)) < 0) {
        perror("Connection failed : send_message_on_standard_socket()");
        close(socket.socket_id);
        exit(EXIT_FAILURE);
    }

    ssize_t bytes_sent = send(socket.socket_id, message, message_size, 0);
    if (bytes_sent < 0) {
        perror("Send failed : send_message_on_standard_socket()");
        close(socket.socket_id);
        exit(EXIT_FAILURE);
    } else if ((size_t)bytes_sent != message_size) {
        fprintf(stderr, "Incomplete send : send_message_on_standard_socket()");
        close(socket.socket_id);
        exit(EXIT_FAILURE);
    }

    close(socket.socket_id);
}

void broadcast_send_message(char* message, char* ip, int port){
    if (strcmp("127.0.0.255", ip) == 0) {
        return; 
    }
        
    Connection socket = broadcast_connection(ip, port);
      
    struct sockaddr_in broadcast_addr;
    memset(&broadcast_addr, 0, sizeof(broadcast_addr));
    broadcast_addr.sin_family = AF_INET;
    broadcast_addr.sin_addr.s_addr = inet_addr(ip);
    broadcast_addr.sin_port = htons(port);

    if (sendto(socket.socket_id, message, strlen(message), 0, (struct sockaddr *)&broadcast_addr, sizeof(broadcast_addr)) < 0) {
        perror("Send failed : broadcast_send_message() ");
        close(socket.socket_id);
        exit(EXIT_FAILURE);
    }

    close(socket.socket_id);
}
