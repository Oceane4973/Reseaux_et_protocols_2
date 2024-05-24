#include "router.h"
#include "utils.c"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <pthread.h>
#include <sys/select.h>
#include <arpa/inet.h>

typedef struct {
    Device *device;
    int port;
} ThreadArg;

Router initRouter(const char *name, int port, Device *devices, int num_devices) {
    Router router;
    router.name = strdup(name); // Allouer et copier le nom du routeur
    router.port = port;
    router.devices = malloc(num_devices * sizeof(Device)); // Allouer de la mémoire pour les dispositifs
    for (int i = 0; i < num_devices; ++i) {
        router.devices[i].interface = strdup(devices[i].interface); // Allouer et copier l'interface
        router.devices[i].ip = strdup(devices[i].ip); // Allouer et copier l'adresse IP
        router.devices[i].mask = devices[i].mask;
    }
    router.num_devices = num_devices;
    return router;
}

void destroyRouter(Router *router) {
    for (int i = 0; i < router->num_devices; ++i) {
        destroyDevice(&(router->devices[i])); // Détruire les dispositifs
    }
    free(router->devices);
    free(router->name); // Libérer la mémoire du nom du routeur

    router->devices = NULL;
    router->name = NULL;
}

void *deviceThread(void *arg) {
    ThreadArg *thread_arg = (ThreadArg *)arg;

    // Accès au périphérique et au port à partir de la structure
    Device *device = thread_arg->device;
    int devicePort = thread_arg->port;

    int tcpSocket, udpSocket;
    struct sockaddr_in tcpAddr, udpAddr;
    int broadcastEnable = 1;
    char buffer[MAX_BUFFER_SIZE];
    ssize_t bytesReceived;
    struct sockaddr_in senderAddr;
    socklen_t addrLen = sizeof(senderAddr);
    fd_set readfds;
    int max_fd;
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
    udpAddr.sin_port = htons(devicePort);

    // Liaison du socket UDP pour écouter sur toutes les interfaces
    if (bind(udpSocket, (struct sockaddr *)&udpAddr, sizeof(udpAddr)) < 0) {
        perror("Broadcast bind failed");
        exit(EXIT_FAILURE);
    }

    printf("Device %s : listening on %s:%d and %s for broadcast \n", device->interface, device->ip, devicePort, broadcast_adrr);

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

            printf("Connection accepted from %s:%d\n", inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port));

            // Réception des données du client
            bytesReceived = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
            if (bytesReceived < 0) {
                perror("Receive failed");
                close(clientSocket);
                continue;
            }
            buffer[bytesReceived] = '\0'; // Assurez-vous que le tampon est nul-terminé

            // Traitement des données reçues (exemple de vérification)
            if (strcmp(buffer, "broadcast") == 0) {
                const char *message = "hello world";
                send(clientSocket, message, strlen(message), 0);
                printf("Sent: %s\n", message);
            }

            // Fermeture de la connexion avec le client
            close(clientSocket);
        }

        /* ------------------------------
        Vérifier les messages broadcast sur le socket UDP
        ------------------------------ */

        if (FD_ISSET(udpSocket, &readfds)) {
            bytesReceived = recvfrom(udpSocket, buffer, sizeof(buffer) - 1, 0, (struct sockaddr *)&senderAddr, &addrLen);
            if (bytesReceived < 0) {
                perror("Receive failed");
                continue;
            }
            buffer[bytesReceived] = '\0'; // Assurez-vous que le tampon est nul-terminé

            printf("Broadcast received from %s:%d: %s\n", inet_ntoa(senderAddr.sin_addr), ntohs(senderAddr.sin_port), buffer);

            // Vous pouvez traiter les messages de broadcast ici
        }
    }

    free(device->interface);
    free(device->ip);
    free(device);
    free(thread_arg);

    close(tcpSocket);
    close(udpSocket);
    pthread_exit(NULL);
}

void startRouter(Router router) {
    pthread_t threads[router.num_devices];

    for (int i = 0; i < router.num_devices; i++) {
        // Allocation mémoire pour la structure ThreadArg
        ThreadArg *thread_arg = malloc(sizeof(ThreadArg));
        if (thread_arg == NULL) {
            perror("Memory allocation failed");
            exit(EXIT_FAILURE);
        }
        
        // Copie du périphérique
        thread_arg->device = malloc(sizeof(Device));
        memcpy(thread_arg->device, &router.devices[i], sizeof(Device));

        // Assignation du port
        thread_arg->port = router.port;

        // Création du thread avec la structure comme argument
        if (pthread_create(&threads[i], NULL, deviceThread, thread_arg) != 0) {
            perror("Thread creation failed");
            exit(EXIT_FAILURE);
        }
    }

    for (int i = 0; i < router.num_devices; i++) {
        if (pthread_join(threads[i], NULL) != 0) {
            perror("Thread join failed");
            exit(EXIT_FAILURE);
        }
    }
}