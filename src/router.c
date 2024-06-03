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
#include "connection.h"
#include "enable_logs.h"

Router initRouter(const char *name, int port, Device *devices, int num_devices) {
    Router router;
    router.name = strdup(name);
    router.port = port;

    router.devices = malloc(num_devices * sizeof(Device));
    if (!router.devices) {
        perror("Failed to allocate memory for router devices");
        exit(EXIT_FAILURE);
    }
    for (int i = 0; i < num_devices; ++i) {
        router.devices[i].interface = strdup(devices[i].interface); 
        router.devices[i].ip = strdup(devices[i].ip);
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
        destroyDevice(&(router->devices[i]));
    }
    destroyRoutingTable(router->routing_table);
    free(router->devices);
    free(router->name);

    router->devices = NULL;
    router->routing_table = NULL;
    router->name = NULL;
}

void *startRouter(void *threadRouterArg) {
    ThreadRouterArg *data = (ThreadRouterArg *)threadRouterArg;
    Router router = data->router;

    pthread_t threads[router.num_devices];

    for (int i = 0; i < router.num_devices; i++) {
        ThreadDevicesArg *thread_arg = malloc(sizeof(ThreadDevicesArg));
        if (thread_arg == NULL) {
            perror("Memory allocation failed");
            exit(EXIT_FAILURE);
        }
        
        thread_arg->device = malloc(sizeof(Device));
        memcpy(thread_arg->device, &router.devices[i], sizeof(Device));

        thread_arg->router = &router;

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
    if (ip_address == NULL || cidr < 0 || cidr > 32) {
        return NULL;
    }

    char* broadcast_address = (char*)malloc(16 * sizeof(char)); 
    if (broadcast_address == NULL) {
        return NULL; 
    }

    char ip_copy[16];
    strcpy(ip_copy, ip_address);

    unsigned long subnet_mask = 0xFFFFFFFFUL << (32 - cidr);

    unsigned long ip = 0;
    char* token = strtok(ip_copy, ".");
    for (int i = 0; i < 4 && token != NULL; i++) {
        ip |= atol(token) << ((3 - i) * 8);
        token = strtok(NULL, ".");
    }

    unsigned long broadcast = (ip & subnet_mask) | (~subnet_mask);

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

        for (int j = 0; j < router->routing_table->num_route; j++) {
            Route *existing_route = &router->routing_table->table[j];

            if (strcmp(existing_route->destination, new_route->destination) == 0 && existing_route->mask == new_route->mask) {
                found = 1;
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

void fill_empty_gateways(Routing_table *routing_table, const char *default_gateway) {
    if (!routing_table || !default_gateway) {
        return;
    }

    for (int i = 0; i < routing_table->num_route; i++) {
        Route *route = &routing_table->table[i];
        free(route->passerelle); 
        route->passerelle = strdup(default_gateway); 
        if (!route->passerelle) {
            perror("Failed to allocate memory for gateway");
            exit(EXIT_FAILURE);
        }
    }
}

char* calculate_network_address(const char *ip_str, int cidr_mask) {
    if (!ip_str) {
        fprintf(stderr, "IP address is NULL\n");
        return NULL;
    }

    struct in_addr ip_addr, network_addr;
    unsigned long mask = (cidr_mask == 0) ? 0 : (~0U << (32 - cidr_mask)) & 0xFFFFFFFF;

    if (inet_pton(AF_INET, ip_str, &ip_addr) <= 0) {
        perror("Invalid IP address");
        return NULL;
    }

    network_addr.s_addr = ip_addr.s_addr & htonl(mask);

    char *network_str = (char*)malloc(INET_ADDRSTRLEN);
    if (!network_str) {
        perror("Memory allocation failed");
        return NULL;
    }

    if (inet_ntop(AF_INET, &network_addr, network_str, INET_ADDRSTRLEN) == NULL) {
        perror("inet_ntop");
        free(network_str);
        return NULL;
    }

    return network_str;
}