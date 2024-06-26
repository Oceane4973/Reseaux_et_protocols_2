#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdio.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#include <sys/select.h>
#include "router.h"
#include "parser.h"
#include "connection.h"
#include "enable_logs.h"
#include "tram.h"

Device initDevice(const char *interface, const char *ip, int mask) {
    Device device;
    device.interface = strdup(interface);
    if (!device.interface) {
        perror("Failed to allocate memory for device interface");
        exit(EXIT_FAILURE);
    }

    device.ip = strdup(ip);
    if (!device.ip) {
        perror("Failed to allocate memory for device IP");
        exit(EXIT_FAILURE);
    }

    device.mask = mask;
    return device;
}

void destroyDevice(Device *device) {
    free(device->interface);
    free(device->ip);
    device->interface = NULL;
    device->ip = NULL;
}

void *send_on_broadcast(void *threadBroadcastArg){
    ThreadBroadcastArg *thread_arg = (ThreadBroadcastArg *)threadBroadcastArg;
    Routing_table *routing_table_send;
    char *routing_table_send_str;
    char* buffer;

    while (1) {
        routing_table_send = copy_routing_table(thread_arg->router->routing_table);
        fill_empty_gateways(routing_table_send, thread_arg->device->ip);
        buffer = routing_table_to_buffer(routing_table_send);
        broadcast_send_message(buffer, thread_arg->broadcast_adrr, BROADCAST_PORT);
        routing_table_send_str = displayRoutingTable(routing_table_send);
        if (enable_logs){
            printf( "%s_%s    Broadcast on %s:%i send its routing table\n%s", thread_arg->router->name, thread_arg->device->interface, thread_arg->broadcast_adrr, BROADCAST_PORT, routing_table_send_str);
        }
        sleep(INTERVAL);
    }
    free(routing_table_send_str);
    free(routing_table_send);

    destroyDevice(thread_arg->device);
    destroyRouter(thread_arg->router);
    free(thread_arg);

    pthread_exit(NULL);
}

void *deviceThread(void *threadDevicesArg) {
    ThreadDevicesArg *thread_arg = (ThreadDevicesArg *)threadDevicesArg;
    char* broadcast_adrr = calculate_broadcast_address(thread_arg->device->ip, thread_arg->device->mask);

    char buffer[MAX_BUFFER_SIZE];
    char buffer_tram[MAX_BUFFER_SIZE];
    ssize_t bytesReceived;
    fd_set readfds;
    int max_fd;

    /* ------------------------------
    socket TCP
    ------------------------------ */

    Connection tcpSocketConnection = standard_connection(thread_arg->device->ip,thread_arg->router->port);
    socklen_t tcp_addr_len = sizeof(tcpSocketConnection.addr);

    if (bind(tcpSocketConnection.socket_id, (struct sockaddr *)&tcpSocketConnection.addr, tcp_addr_len) < 0) {
        perror("tcpSocket bind failed");
        pthread_exit(NULL);
    }

    if (listen(tcpSocketConnection.socket_id, 5) < 0) {
        perror("Listen failed");
        pthread_exit(NULL);
    }

    /* ------------------------------
    socket UDP
    ------------------------------ */
    Connection udpSocketConnection = broadcast_connection(broadcast_adrr, BROADCAST_PORT);
    socklen_t udp_addr_len = sizeof(udpSocketConnection.addr); 
    
    int reuse = 1;
    if (setsockopt(udpSocketConnection.socket_id, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) == -1) {
        perror("setsockopt for SO_REUSEADDR failed");
        pthread_exit(NULL);
    }

    if (bind(udpSocketConnection.socket_id, (struct sockaddr *)&udpSocketConnection.addr, udp_addr_len) < 0) {
        perror("Broadcast bind failed");
        pthread_exit(NULL);
    }

    printf("%s_%s    Listening on %s:%d and %s:%i for broadcast \n", thread_arg->router->name, thread_arg->device->interface, thread_arg->device->ip, thread_arg->router->port, broadcast_adrr, BROADCAST_PORT);
    
    /* ------------------------------
    Broadcast
    ------------------------------ */
    pthread_t thread;
    ThreadBroadcastArg *thread_broadcast_arg = malloc(sizeof(ThreadBroadcastArg));
    thread_broadcast_arg->device = thread_arg->device;
    thread_broadcast_arg->router = thread_arg->router;
    thread_broadcast_arg->broadcast_adrr = broadcast_adrr;
    thread_broadcast_arg->port = BROADCAST_PORT;
    pthread_create(&thread, NULL, send_on_broadcast, thread_broadcast_arg);

    while (1) {
        FD_ZERO(&readfds);
        FD_SET(tcpSocketConnection.socket_id, &readfds);
        FD_SET(udpSocketConnection.socket_id, &readfds);

        max_fd = (tcpSocketConnection.socket_id > udpSocketConnection.socket_id) ? tcpSocketConnection.socket_id : udpSocketConnection.socket_id;

        int activity = select(max_fd + 1, &readfds, NULL, NULL, NULL);
        if (activity < 0) {
            perror("select");
            close(tcpSocketConnection.socket_id);
            close(udpSocketConnection.socket_id);
            pthread_exit(NULL);
        }

        /* ------------------------------
        connection : socket TCP
        ------------------------------ */

        if (FD_ISSET(tcpSocketConnection.socket_id, &readfds)) {
            int clientSocket;
            struct sockaddr_in clientAddr;
            socklen_t clientAddrLen = sizeof(clientAddr);

            if ((clientSocket = accept(tcpSocketConnection.socket_id, (struct sockaddr *)&clientAddr, &clientAddrLen)) < 0) {
                perror("Accept failed");
                pthread_exit(NULL);
            }

            if (enable_logs){
                printf("%s_%s    Connection accepted from %s:%d\n", thread_arg->router->name, thread_arg->device->interface, inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port));
            }

            bytesReceived = recv(clientSocket, buffer_tram, sizeof(buffer_tram) - 1, 0);
            if (bytesReceived < 0) {
                perror("Receive failed");
                close(clientSocket);
                continue;
            }
            
            buffer_tram[bytesReceived] = '\0'; 
            Tram *tram = buffer_to_tram(buffer_tram);
            if (tram != NULL){
                char *tram_str = display_tram(tram);
                printf("%s_%s    Receveid: %s\n", thread_arg->router->name, thread_arg->device->interface, tram_str);
                if (strcmp(tram->destination, thread_arg->device->ip) ==0 ){
                    printf( "-------------------------------------------------\n"
                    "%s_%s    Received message: \n%s\n"
                    "-------------------------------------------------\n", thread_arg->router->name, thread_arg->device->interface, tram->message);
                } else {
                    for (int i = 0; i < thread_arg->router->routing_table->num_route; ++i) {
                        Route *route = &thread_arg->router->routing_table->table[i];

                        if (strcmp(calculate_network_address(tram->destination, route->mask), route->destination)==0) {
                            char* gateway= route->passerelle;
                            int port= thread_arg->router->port;
                            if (strcmp(gateway, "")==0){
                                gateway = route->interface;
                            }
                            if (strcmp(thread_arg->device->ip, gateway)==0){
                                gateway = tram->destination;
                                port = tram->port;
                            }
                            printf("%s_%s    Message received for another network. Forwarding via gateway: %s:%i\n", thread_arg->router->name, thread_arg->device->interface, gateway, port);
                            char* message = tram_to_buffer(tram);
                            send_message_on_standard_socket(message, strlen(message), gateway, port);
                            printf("%s_%s    Send message to %s\n", thread_arg->router->name, thread_arg->device->interface, gateway);
                        }
                    }
                }

                free(tram_str);
                destroyTram(tram);
            }
            close(clientSocket);
        }

        /* ------------------------------
        connection : socket UDP
        ------------------------------ */

        if (FD_ISSET(udpSocketConnection.socket_id, &readfds)) {

            int bytesReceived = recvfrom(udpSocketConnection.socket_id, buffer, BUFFER_SIZE - 1, 0, (struct sockaddr *)&udpSocketConnection.addr, &udp_addr_len);
            if (bytesReceived < 0) {
                perror("bytesReceived is NULL");
                close(tcpSocketConnection.socket_id);
                close(udpSocketConnection.socket_id);
                exit(EXIT_FAILURE);
            }
            buffer[bytesReceived] = '\0';
            FILE *file = fmemopen((void *)buffer, bytesReceived, "r"); 
            if (!file) {
                perror("Failed to open memory as file");
                close(tcpSocketConnection.socket_id);
                close(udpSocketConnection.socket_id);
                exit(EXIT_FAILURE);
            }

            Routing_table *routing_table = parse_yaml_file_to_routing_table(file);
            fclose(file); 
            if (routing_table == NULL) {
                fprintf(stderr, "Failed to parse YAML routing table\n");
                close(tcpSocketConnection.socket_id);
                close(udpSocketConnection.socket_id);
                exit(EXIT_FAILURE);
            }

            updateRoutingTable(thread_arg->router, routing_table);
            if (enable_logs){
                char *routing_table_str = displayRoutingTable(thread_arg->router->routing_table);
                printf( "%s_%s    Broadcast on %s:%i received a routing table\n"
                        "%s         Updated routing table :  \n%s", thread_arg->router->name, thread_arg->device->interface, broadcast_adrr, BROADCAST_PORT, thread_arg->router->name, routing_table_str);
                free(routing_table_str);
            }
            destroyRoutingTable(routing_table);
        }
    }

    pthread_join(thread, NULL);

    destroyDevice(thread_arg->device);
    destroyRouter(thread_arg->router);
    free(thread_arg);

    close(tcpSocketConnection.socket_id);
    close(udpSocketConnection.socket_id);
    pthread_exit(NULL);
}
