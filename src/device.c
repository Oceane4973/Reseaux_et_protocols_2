#include "device.h"
#include <stdlib.h>
#include <string.h>

Device initDevice(const char *interface, const char *ip, int mask) {
    Device device;

    // Alloue de la mémoire pour le nom de l'interface et copie la chaîne
    device.interface = (char *)malloc(strlen(interface) + 1);
    strcpy(device.interface, interface);

    // Alloue de la mémoire pour l'adresse IP et copie la chaîne
    device.ip = (char *)malloc(strlen(ip) + 1);
    strcpy(device.ip, ip);

    // Initialise le masque
    device.mask = mask;

    return device;
}

void destroyDevice(Device *device) {
    free(device->interface);
    free(device->ip);
    device->interface = NULL;
    device->ip = NULL;
}