#include <stdio.h>
#include "router.h"

int main() {
    // Ouverture du fichier config.yaml en mode lecture
    FILE *file = fopen("config.yaml", "rb");
    if (!file) {
        fprintf(stderr, "Erreur lors de l'ouverture du fichier config.yaml\n");
        return 1;
    }



    // Exemple de création d'un routeur avec deux appareils
    Device devices[] = {
        {"eth0", "127.0.0.1", 24},
        {"eth1", "192.1.1.2", 24}
    };
    Router my_router = {"R1", 8000, devices, 2};
    // Démarrage du routeur
    startRouter(my_router);

    return 0;
}