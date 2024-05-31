#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "tram.h"

void destroyTram(Tram *tram) {
    if (tram) {
        // Libérer la mémoire allouée pour les champs de la structure
        free(tram->destination);
        free(tram->origin);
        free(tram->message);
        
        // Libérer la mémoire de la structure Tram elle-même
        free(tram);
    }
}

char* tram_to_buffer(const Tram *tram) {
    if (!tram) {
        fprintf(stderr, "Tram is NULL\n");
        return NULL;
    }

    // Allouer de la mémoire pour le buffer
    char *buffer = (char*)malloc(MAX_BUFFER_SIZE);
    if (!buffer) {
        perror("Memory allocation failed");
        exit(EXIT_FAILURE);
    }

    // Formatage du Tram dans le buffer
    snprintf(buffer, MAX_BUFFER_SIZE, "Destination: %s\nOrigin: %s\nPort: %d\nMessage: %s\n",
             tram->destination ? tram->destination : "NULL",
             tram->origin ? tram->origin : "NULL",
             tram->port,
             tram->message ? tram->message : "NULL");

    return buffer;
}

Tram* buffer_to_tram(const char *buffer) {
    if (!buffer) {
        fprintf(stderr, "Buffer is NULL\n");
        return NULL;
    }

    // Allouer de la mémoire pour un nouvel objet Tram
    Tram *tram = (Tram*)malloc(sizeof(Tram));
    if (!tram) {
        perror("Memory allocation failed");
        exit(EXIT_FAILURE);
    }

    // Initialiser les pointeurs de la structure Tram à NULL
    tram->destination = NULL;
    tram->origin = NULL;
    tram->message = NULL;

    // Analyser le buffer pour extraire les informations du Tram
    int result = sscanf(buffer, "Destination: %ms\nOrigin: %ms\nPort: %d\nMessage: %ms\n",
                       &tram->destination, &tram->origin, &tram->port, &tram->message);

    // Vérifier si la lecture a réussi
    /**if (result != 4) {
        fprintf(stderr, "Failed to parse Tram from buffer\n");
        free(tram);  // Libérer la mémoire allouée
        return NULL;
    }**/

    return tram;
}

char* display_tram(Tram *tram) {
    if (!tram) {
        return strdup("Tram is NULL.\n");
    }

    // Taille initiale du buffer
    size_t buffer_size = 1024;
    char *result = (char *)malloc(buffer_size);
    if (!result) {
        perror("Failed to allocate memory");
        exit(EXIT_FAILURE);
    }
    result[0] = '\0'; // Assurez-vous que la chaîne est vide pour commencer

    // Formater les informations de tram dans une chaîne
    char info[256];
    snprintf(info, sizeof(info),
        "{ Destination: %s, Origin: %s, Port: %d, Message: %s }\n",
        tram->destination, tram->origin, tram->port, tram->message);

    // Vérifier si la taille de la chaîne résultante dépasse la taille du buffer
    if (strlen(result) + strlen(info) + 1 > buffer_size) {
        // Si oui, doubler la taille du buffer
        buffer_size *= 2;
        result = (char *)realloc(result, buffer_size);
        if (!result) {
            perror("Failed to reallocate memory");
            exit(EXIT_FAILURE);
        }
    }

    // Concaténer les informations de tram avec la chaîne résultante
    strcat(result, info);

    return result;
}