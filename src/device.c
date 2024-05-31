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

    // Accès au périphérique et au port à partir de la structure
    char buffer[MAX_BUFFER_SIZE];
    ssize_t bytesReceived;
    fd_set readfds;
    int max_fd;


    /* ------------------------------
    Création du socket TCP
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
    Création du socket UDP pour les messages broadcast
    ------------------------------ */
    Connection udpSocketConnection = broadcast_connection(broadcast_adrr, BROADCAST_PORT);
    socklen_t udp_addr_len = sizeof(udpSocketConnection.addr); // Obtenez la taille de la structure d'adresse

    // Activation de l'option SO_REUSEADDR pour le socket UDP
    int reuse = 1;
    if (setsockopt(udpSocketConnection.socket_id, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) == -1) {
        perror("setsockopt for SO_REUSEADDR failed");
        pthread_exit(NULL);
    }

    // Liaison du socket UDP pour écouter sur toutes les interfaces
    if (bind(udpSocketConnection.socket_id, (struct sockaddr *)&udpSocketConnection.addr, udp_addr_len) < 0) {
        perror("Broadcast bind failed");
        pthread_exit(NULL);
    }

    printf("%s_%s    Listening on %s:%d and %s:%i for broadcast \n", thread_arg->router->name, thread_arg->device->interface, thread_arg->device->ip, thread_arg->router->port, broadcast_adrr, BROADCAST_PORT);
    
    /* ------------------------------
    Création du socket UDP pour les messages broadcast
    ------------------------------ */
    pthread_t thread;
    ThreadBroadcastArg *thread_broadcast_arg = malloc(sizeof(ThreadBroadcastArg));
    thread_broadcast_arg->device = thread_arg->device;
    thread_broadcast_arg->router = thread_arg->router;
    thread_broadcast_arg->broadcast_adrr = broadcast_adrr;
    thread_broadcast_arg->port = BROADCAST_PORT;
    pthread_create(&thread, NULL, send_on_broadcast, thread_broadcast_arg);

    // Boucle principale pour la réception des messages
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
        Vérifier les connexions entrantes sur le socket TCP
        ------------------------------ */

        if (FD_ISSET(tcpSocketConnection.socket_id, &readfds)) {
            int clientSocket;
            struct sockaddr_in clientAddr;
            socklen_t clientAddrLen = sizeof(clientAddr);

            // Acceptation de la connexion entrante
            if ((clientSocket = accept(tcpSocketConnection.socket_id, (struct sockaddr *)&clientAddr, &clientAddrLen)) < 0) {
                perror("Accept failed");
                pthread_exit(NULL);
            }

            //if (enable_logs){
                printf("%s_%s    Connection accepted from %s:%d\n", thread_arg->router->name, thread_arg->device->interface, inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port));
            //}

            // Réception des données du client
            bytesReceived = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
            if (bytesReceived < 0) {
                perror("Receive failed");
                close(clientSocket);
                continue;
            }
            buffer[bytesReceived] = '\0'; // Assurez-vous que le tampon est nul-terminé

            Tram *tram = buffer_to_tram(buffer);
            
            char *tram_str = display_tram(tram);

            printf("%s_%s    Receveid: %s\n", thread_arg->router->name, thread_arg->device->interface, tram_str);
            
            free(tram_str);
            destroyTram(tram);
            
            //printf("%s_%s    Sent: \n%s\n", thread_arg->router->name,device->interface, message);
            //}

            // Fermeture de la connexion avec le client
            close(clientSocket);
        }

        /* ------------------------------
        Vérifier les messages broadcast sur le socket UDP
        ------------------------------ */

        if (FD_ISSET(udpSocketConnection.socket_id, &readfds)) {

            //GESTION RECEPTION
            
            int bytesReceived = recvfrom(udpSocketConnection.socket_id, buffer, BUFFER_SIZE - 1, 0, (struct sockaddr *)&udpSocketConnection.addr, &udp_addr_len);
            if (bytesReceived < 0) {
                perror("bytesReceived is NULL");
                //close(tcpSocket);
                //close(udpSocket);
                //exit(EXIT_FAILURE);
            }
            buffer[bytesReceived] = '\0'; // Ajout du caractère de fin de chaîne au buffer
            FILE *file = fmemopen((void *)buffer, bytesReceived, "r"); // Utilisation de bytesReceived pour la taille du buffer
            if (!file) {
                perror("Failed to open memory as file");
                //close(tcpSocket);
                //close(udpSocket);
                //exit(EXIT_FAILURE);
            }

            // Parser le fichier YAML en table de routage
            Routing_table *routing_table = parse_yaml_file_to_routing_table(file);
            
            //C'est celui qui envoie qui remplit ses passerelles ! 
            //fill_empty_gateways(routing_table, thread_arg->device->ip);

            fclose(file); // Fermeture du fichier après utilisation

            if (routing_table == NULL) {
                fprintf(stderr, "Failed to parse YAML routing table\n");
                //close(tcpSocket);
                //close(udpSocket);
                //exit(EXIT_FAILURE);
            }

            // Afficher la table de routage reçue
            /**char *routing_table_str = displayRoutingTable(routing_table);
            printf("%s_%s    Broadcast on %s:%i received :  \n%s", thread_arg->router->name, device->interface, broadcast_adrr, BROADCAST_PORT, routing_table_str);
            free(routing_table_str);**/

            // Mettre à jour la table de routage du routeur
            updateRoutingTable(thread_arg->router, routing_table);

            // Libérer la mémoire allouée pour la table de routage

            // Réafficher la table de routage mise à jour
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
