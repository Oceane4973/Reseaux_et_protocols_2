#include <stdio.h>


// Structure pour stocker les informations sur un appareil
typedef struct {
    char *interface;
    char *ip;
    int mask;
} Device;

// Structure pour stocker les informations sur un routeur
typedef struct {
    char *name;
    int port;
    Device *devices;
    int num_devices;
} Router;

int main() {
    // Ouverture du fichier config.yaml en mode lecture
    FILE *file = fopen("config.yaml", "rb");
    if (!file) {
        fprintf(stderr, "Erreur lors de l'ouverture du fichier config.yaml\n");
        return 1;
    }
}