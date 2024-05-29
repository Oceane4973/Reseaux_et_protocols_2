#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <pthread.h>
#include <sys/select.h>
#include "router.h"
#include "parser.h"

Router initRouter(const char *name, int port, Device *devices, int num_devices) {
    Router router;
    router.name = strdup(name); // Allouer et copier le nom du routeur
    router.port = port;

    // Allouer de la mémoire pour les dispositifs et copier les données
    router.devices = malloc(num_devices * sizeof(Device));
    if (!router.devices) {
        perror("Failed to allocate memory for router devices");
        exit(EXIT_FAILURE);
    }
    for (int i = 0; i < num_devices; ++i) {
        router.devices[i].interface = strdup(devices[i].interface); // Allouer et copier l'interface
        router.devices[i].ip = strdup(devices[i].ip); // Allouer et copier l'adresse IP
        router.devices[i].mask = devices[i].mask;
    }
    router.num_devices = num_devices;

    char routing_table_path[MAX_PATH_LENGTH];
    sprintf(routing_table_path, "%s%s/routing_table.yaml", GLOBAL_ROUTING_TABLE_PATH, name);
    
    router.routing_table = initRoutingTable(routing_table_path);

    return router;
}

void destroyRouter(Router *router) {
    for (int i = 0; i < router->num_devices; ++i) {
        destroyDevice(&(router->devices[i])); // Détruire les dispositifs
    }
    destroyRoutingTable(router->routing_table);
    free(router->devices);
    free(router->name); // Libérer la mémoire du nom du routeur

    router->devices = NULL;
    router->routing_table = NULL;
    router->name = NULL;
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
            const char *message = "hello world";
            send(clientSocket, message, strlen(message), 0);
            printf("%s_%s    Sent: \n%s\n", thread_arg->router->name,device->interface, message);
            //}

            // Fermeture de la connexion avec le client
            close(clientSocket);
        }

        /* ------------------------------
        Vérifier les messages broadcast sur le socket UDP
        ------------------------------ */

        if (FD_ISSET(udpSocket, &readfds)) {
            
            int bytesReceived = recvfrom(udpSocket, buffer, BUFFER_SIZE - 1, 0, (struct sockaddr *)&udpAddr, &addrLen);
            if (bytesReceived < 0) {
                close(tcpSocket);
                close(udpSocket);
                exit(EXIT_FAILURE);
            }
            buffer[bytesReceived] = '\0'; // Ajout du caractère de fin de chaîne au buffer
            FILE *file = fmemopen((void *)buffer, bytesReceived, "r"); // Utilisation de bytesReceived pour la taille du buffer
            if (!file) {
                perror("Failed to open memory as file");
                close(tcpSocket);
                close(udpSocket);
                exit(EXIT_FAILURE);
            }

            // Parser le fichier YAML en table de routage
            Routing_table *routing_table = parse_yaml_file_to_routing_table(file);
            fclose(file); // Fermeture du fichier après utilisation

            if (routing_table == NULL) {
                fprintf(stderr, "Failed to parse YAML routing table\n");
                close(tcpSocket);
                close(udpSocket);
                exit(EXIT_FAILURE);
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

    destroyDevice(thread_arg->device);
    destroyRouter(thread_arg->router);
    free(thread_arg);

    close(tcpSocket);
    close(udpSocket);
    pthread_exit(NULL);
}

void *startRouter(void *threadRouterArg) {
    ThreadRouterArg *data = (ThreadRouterArg *)threadRouterArg;
    Router router = data->router;

    pthread_t threads[router.num_devices];

    for (int i = 0; i < router.num_devices; i++) {
        // Allocation mémoire pour la structure ThreadArg
        ThreadDevicesArg *thread_arg = malloc(sizeof(ThreadDevicesArg));
        if (thread_arg == NULL) {
            perror("Memory allocation failed");
            exit(EXIT_FAILURE);
        }
        
        // Copie du périphérique
        thread_arg->device = malloc(sizeof(Device));
        memcpy(thread_arg->device, &router.devices[i], sizeof(Device));

        // Assignation du port
        thread_arg->router = &router;

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

    destroyRouter(&router);
    free(data);

    pthread_exit(NULL);
}

char* calculate_broadcast_address(const char* ip_address, const int cidr) {
    // Vérification des arguments
    if (ip_address == NULL || cidr < 0 || cidr > 32) {
        return NULL;
    }

    // Allocation de mémoire pour l'adresse de diffusion
    char* broadcast_address = (char*)malloc(16 * sizeof(char)); // 16 pour une adresse IP IPv4 maximale

    if (broadcast_address == NULL) {
        return NULL; // Échec de l'allocation de mémoire
    }

    // Copie de l'adresse IP dans une variable modifiable
    char ip_copy[16]; // 16 pour une adresse IP IPv4 maximale
    strcpy(ip_copy, ip_address);

    // Calcul du masque de sous-réseau
    unsigned long subnet_mask = 0xFFFFFFFFUL << (32 - cidr);

    // Conversion de l'adresse IP en entier sans le CIDR
    unsigned long ip = 0;
    char* token = strtok(ip_copy, ".");
    for (int i = 0; i < 4 && token != NULL; i++) {
        ip |= atol(token) << ((3 - i) * 8);
        token = strtok(NULL, ".");
    }

    // Calcul de l'adresse de diffusion
    unsigned long broadcast = (ip & subnet_mask) | (~subnet_mask);

    // Conversion de l'adresse de diffusion en format de chaîne
    sprintf(broadcast_address, "%lu.%lu.%lu.%lu",
            (broadcast >> 24) & 0xFF,
            (broadcast >> 16) & 0xFF,
            (broadcast >> 8) & 0xFF,
            broadcast & 0xFF);

    return broadcast_address;
}

void updateRoutingTable(Router *router, Routing_table *routing_table) {
    if (!router || !router->routing_table || !routing_table) {
        return;
    }

    for (int i = 0; i < routing_table->num_route; i++) {
        Route *new_route = &routing_table->table[i];
        new_route->distance++;
        int found = 0;

        // Check if the destination is already in the router's routing table
        for (int j = 0; j < router->routing_table->num_route; j++) {
            Route *existing_route = &router->routing_table->table[j];

            if (strcmp(existing_route->destination, new_route->destination) == 0 && existing_route->mask == new_route->mask) {
                found = 1;
                // Compare distances and update if the new distance is shorter
                if (new_route->distance < existing_route->distance) {
                    existing_route->distance = new_route->distance;
                    if (existing_route->passerelle) free(existing_route->passerelle);
                    if (existing_route->interface) free(existing_route->interface);
                    existing_route->passerelle = strdup(new_route->passerelle);
                    existing_route->interface = strdup(new_route->interface);
                }
                break;
            }
        }

        // If the destination is not found, add the new route
        if (!found) {
            router->routing_table->table = realloc(router->routing_table->table, (router->routing_table->num_route + 1) * sizeof(Route));
            if (!router->routing_table->table) {
                perror("Failed to reallocate memory for routing table");
                exit(EXIT_FAILURE);
            }
            router->routing_table->table[router->routing_table->num_route].destination = strdup(new_route->destination);
            router->routing_table->table[router->routing_table->num_route].mask = new_route->mask;
            router->routing_table->table[router->routing_table->num_route].passerelle = strdup(new_route->passerelle);
            router->routing_table->table[router->routing_table->num_route].interface = strdup(new_route->interface);
            router->routing_table->table[router->routing_table->num_route].distance = new_route->distance;
            router->routing_table->num_route++;
        }
    }
}