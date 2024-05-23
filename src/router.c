#include "router.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <pthread.h>

typedef struct {
    Device *device;
    int port;
} ThreadArg;

Router initRouter(const char *name, int port, Device *devices, int num_devices) {
    Router router;

    router.name = (char *)malloc(strlen(name) + 1);
    strcpy(router.name, name);

    router.port = port;

    router.devices = (Device *)malloc(sizeof(Device) * num_devices);
    for (int i = 0; i < num_devices; i++) {
        router.devices[i] = devices[i];
    }
    router.num_devices = num_devices;

    return router;
}

void destroyRouter(Router *router) {
    if (router == NULL) {
        return;
    }

    free(router->name);

    for (int i = 0; i < router->num_devices; i++) {
        free(router->devices[i].interface);
        free(router->devices[i].ip);
    }

    free(router->devices);
    free(router);
}

void *deviceThread(void *arg) {
    ThreadArg *thread_arg = (ThreadArg *)arg;

    // Accès au périphérique et au port à partir de la structure
    Device *device = thread_arg->device;
    int devicePort = thread_arg->port;

    int deviceSocket;
    struct sockaddr_in deviceAddr;

    // Création de la socket TCP
    if ((deviceSocket = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Configuration de l'adresse du périphérique
    deviceAddr.sin_family = AF_INET;
    deviceAddr.sin_addr.s_addr = inet_addr(device->ip);
    deviceAddr.sin_port = htons(0); // Sélection automatique du port

    // Liaison de la socket à une adresse IP spécifiée et à un port aléatoire
    if (bind(deviceSocket, (struct sockaddr *)&deviceAddr, sizeof(deviceAddr)) < 0) {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }

    // Écoute des connexions entrantes
    if (listen(deviceSocket, 5) < 0) {
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }

    printf("Device %s listening on port %d\n", device->interface, devicePort);

    while (1) {
        int clientSocket;
        struct sockaddr_in clientAddr;
        int clientAddrLen = sizeof(clientAddr);

        // Acceptation de la connexion entrante
        if ((clientSocket = accept(deviceSocket, (struct sockaddr *)&clientAddr, (socklen_t *)&clientAddrLen)) < 0) {
            perror("Accept failed");
            exit(EXIT_FAILURE);
        }

        printf("Connection accepted from %s:%d\n", inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port));

        // Réception des données du client
        char buffer[MAX_BUFFER_SIZE];
        ssize_t bytesReceived = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
        if (bytesReceived < 0) {
            perror("Receive failed");
            close(clientSocket);
            continue;
        }
        buffer[bytesReceived] = '\0'; // Assurez-vous que le tampon est nul-terminé
        //printf("Received: %s\n", buffer);

        // Vérification de l'événement de diffusion
        if (strcmp(buffer, "broadcast") == 0) {
            const char *message = "hello world";
            send(clientSocket, message, strlen(message), 0);
            printf("Sent: %s\n", message);
        }
        // Traitement des données reçues (non implémenté dans cet exemple)
        // Vous pouvez ajouter ici le code pour traiter les paquets de données et router les informations.

        // Fermeture de la connexion avec le client
        close(clientSocket);
    }
    free(device);
    free(thread_arg);
    
    close(deviceSocket);
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
