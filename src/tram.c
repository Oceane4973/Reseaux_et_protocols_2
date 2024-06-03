#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "tram.h"

void destroyTram(Tram *tram) {
    if (tram) {
        free(tram->destination);
        free(tram->origin);
        free(tram->message);
        
        free(tram);
    }
}

Tram* buffer_to_tram(const char *buffer) {
    if (!buffer) {
        fprintf(stderr, "Buffer is NULL\n");
        return NULL;
    }

    Tram *tram = (Tram*)malloc(sizeof(Tram));
    if (!tram) {
        perror("Memory allocation failed");
        exit(EXIT_FAILURE);
    }

    tram->destination = NULL;
    tram->origin = NULL;
    tram->message = NULL;

    sscanf(buffer, "Destination: %ms\nOrigin: %ms\nPort: %d\nMessage: %ms\n", &tram->destination, &tram->origin, &tram->port, &tram->message);

    return tram;
}

char* tram_to_buffer(const Tram *tram) {
    if (!tram) {
        fprintf(stderr, "Tram is NULL\n");
        return NULL;
    }

    char *buffer = (char*)malloc(MAX_BUFFER_SIZE);
    if (!buffer) {
        perror("Memory allocation failed");
        exit(EXIT_FAILURE);
    }

    buffer[0] = '\0';

    snprintf(buffer, MAX_BUFFER_SIZE, "Destination: %s\nOrigin: %s\nPort: %d\nMessage: %s\n",
             tram->destination ? tram->destination : "NULL",
             tram->origin ? tram->origin : "NULL",
             tram->port,
             tram->message ? tram->message : "NULL");

    return buffer;
}

char* display_tram(Tram *tram) {
    if (!tram) {
        return strdup("Tram is NULL.\n");
    }

    size_t buffer_size = 1024;
    char *result = (char *)malloc(buffer_size);
    if (!result) {
        perror("Failed to allocate memory");
        exit(EXIT_FAILURE);
    }
    result[0] = '\0'; 
    char info[256];
    snprintf(info, sizeof(info),
        "{ Destination: %s, Origin: %s, Port: %d, Message: %s }",
        tram->destination, tram->origin, tram->port, tram->message);

    if (strlen(result) + strlen(info) + 1 > buffer_size) {
        buffer_size *= 2;
        result = (char *)realloc(result, buffer_size);
        if (!result) {
            perror("Failed to reallocate memory");
            exit(EXIT_FAILURE);
        }
    }
    strcat(result, info);

    return result;
}