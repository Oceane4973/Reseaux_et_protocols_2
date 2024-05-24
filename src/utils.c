
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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