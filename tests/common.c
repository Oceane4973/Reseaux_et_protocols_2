#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

// Structure pour une entrée de la table de routage
struct RoutingTableEntry {
    char destinationIP[16];
    char subnetMask[16];
    char gateway[16];
    char interface[10];
    int distance;
};

// Fonction pour transmettre le fichier sur le réseau via la passerelle correspondante
void transmitFile(char *filename, char *destinationIP, char *interface) {
    printf("Transmitting file %s to %s via interface %s\n", filename, destinationIP, interface);
    // Ici, vous pouvez implémenter la transmission du fichier via les sockets
    // Exemple : création d'une connexion TCP avec le serveur et envoi du fichier
}

// Fonction pour vérifier si le réseau de destination est connu dans la table de routage
int isDestinationKnown(char *destinationIP, struct RoutingTableEntry *routingTable, int tableSize) {
    for (int i = 0; i < tableSize; i++) {
        if (strcmp(destinationIP, routingTable[i].destinationIP) == 0) {
            return i; // Retourne l'index de l'entrée dans la table de routage si le réseau est connu
        }
    }
    return -1; // Retourne -1 si le réseau est inconnu
}
