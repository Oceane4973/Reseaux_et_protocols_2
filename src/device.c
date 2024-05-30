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
    while (1) {
        routing_table_send = copy_routing_table(thread_arg->router->routing_table);
        fill_empty_gateways(routing_table_send, thread_arg->device->ip);
        routing_table_send_str = displayRoutingTable(routing_table_send);
        broadcast_send_message(routing_table_send_str, thread_arg->broadcast_adrr, BROADCAST_PORT);
        printf( "%s_%s    Broadcast on %s:%i send its routing table\n%s", thread_arg->router->name, thread_arg->device->interface, thread_arg->broadcast_adrr, BROADCAST_PORT, routing_table_send_str);
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

    // Accès au périphérique et au port à partir de la structure
    Device *device = thread_arg->device;
    int devicePort = thread_arg->router->port;

    int tcpSocket, udpSocket;
    struct sockaddr_in tcpAddr, udpAddr;
    int broadcastEnable = 1;
    char buffer[MAX_BUFFER_SIZE];
    ssize_t bytesReceived;
    fd_set readfds;
    int max_fd;
    socklen_t addrLen = sizeof(udpAddr);
    char* broadcast_adrr = calculate_broadcast_address(device->ip, device->mask);

    /* ------------------------------
    Création du socket TCP
    ------------------------------ */

    if ((tcpSocket = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Configuration de l'adresse du périphérique pour TCP
    tcpAddr.sin_family = AF_INET;
    tcpAddr.sin_addr.s_addr = inet_addr(device->ip);
    tcpAddr.sin_port = htons(devicePort);

    // Liaison du socket TCP à une adresse IP spécifiée et à un port donné
    if (bind(tcpSocket, (struct sockaddr *)&tcpAddr, sizeof(tcpAddr)) < 0) {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }

    // Écoute des connexions entrantes sur le socket TCP
    if (listen(tcpSocket, 5) < 0) {
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }

    /* ------------------------------
    Création du socket UDP pour les messages broadcast
    ------------------------------ */
    
    if ((udpSocket = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Activation de l'option SO_BROADCAST pour le socket UDP
    if (setsockopt(udpSocket, SOL_SOCKET, SO_BROADCAST, &broadcastEnable, sizeof(broadcastEnable)) == -1) {
        perror("setsockopt for SO_BROADCAST failed");
        exit(EXIT_FAILURE);
    }

    // Activation de l'option SO_REUSEADDR pour le socket UDP
    int reuse = 1;
    if (setsockopt(udpSocket, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) == -1) {
        perror("setsockopt for SO_REUSEADDR failed");
        exit(EXIT_FAILURE);
    }

    // Configuration de l'adresse pour le socket UDP
    udpAddr.sin_family = AF_INET;
    udpAddr.sin_addr.s_addr = inet_addr(broadcast_adrr);
    udpAddr.sin_port = htons(BROADCAST_PORT);

    // Liaison du socket UDP pour écouter sur toutes les interfaces
    if (bind(udpSocket, (struct sockaddr *)&udpAddr, sizeof(udpAddr)) < 0) {
        perror("Broadcast bind failed");
        exit(EXIT_FAILURE);
    }

    printf("%s_%s    Listening on %s:%d and %s:%i for broadcast \n", thread_arg->router->name, device->interface, device->ip, devicePort, broadcast_adrr, BROADCAST_PORT);
    
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
        FD_SET(tcpSocket, &readfds);
        FD_SET(udpSocket, &readfds);

        max_fd = (tcpSocket > udpSocket) ? tcpSocket : udpSocket;

        int activity = select(max_fd + 1, &readfds, NULL, NULL, NULL);
        if (activity < 0) {
            perror("select");
            close(tcpSocket);
            close(udpSocket);
            exit(EXIT_FAILURE);
        }

        /* ------------------------------
        Vérifier les connexions entrantes sur le socket TCP
        ------------------------------ */

        if (FD_ISSET(tcpSocket, &readfds)) {
            int clientSocket;
            struct sockaddr_in clientAddr;
            socklen_t clientAddrLen = sizeof(clientAddr);

            // Acceptation de la connexion entrante
            if ((clientSocket = accept(tcpSocket, (struct sockaddr *)&clientAddr, &clientAddrLen)) < 0) {
                perror("Accept failed");
                exit(EXIT_FAILURE);
            }

            printf("%s_%s    Connection accepted from %s:%d\n", thread_arg->router->name, device->interface, inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port));

            // Réception des données du client
            bytesReceived = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
            if (bytesReceived < 0) {
                perror("Receive failed");
                close(clientSocket);
                continue;
            }
            buffer[bytesReceived] = '\0'; // Assurez-vous que le tampon est nul-terminé

            // Traitement des données reçues (exemple de vérification)
            //if (strcmp(buffer, "broadcast") == 0) {
            //const char *message = "hello world";
            //send(clientSocket, message, strlen(message), 0);
            //printf("%s_%s    Sent: \n%s\n", thread_arg->router->name,device->interface, message);
            //}

            // Fermeture de la connexion avec le client
            close(clientSocket);
        }

        /* ------------------------------
        Vérifier les messages broadcast sur le socket UDP
        ------------------------------ */

        if (FD_ISSET(udpSocket, &readfds)) {

            //GESTION RECEPTION
            
            int bytesReceived = recvfrom(udpSocket, buffer, BUFFER_SIZE - 1, 0, (struct sockaddr *)&udpAddr, &addrLen);
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
            destroyRoutingTable(routing_table);

            // Réafficher la table de routage mise à jour
            char *routing_table_str = displayRoutingTable(thread_arg->router->routing_table);
            printf( "%s_%s    Broadcast on %s:%i received a routing table\n"
                    "%s         Updated routing table :  \n%s", thread_arg->router->name, device->interface, broadcast_adrr, BROADCAST_PORT, thread_arg->router->name, routing_table_str);
            free(routing_table_str);
        }
    }

    pthread_join(thread, NULL);

    destroyDevice(thread_arg->device);
    destroyRouter(thread_arg->router);
    free(thread_arg);

    close(tcpSocket);
    close(udpSocket);
    pthread_exit(NULL);
}
