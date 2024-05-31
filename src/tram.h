#ifndef TRAM_H
#define TRAM_H

typedef struct {
    char *destination;
    char *origin;
    int port;
    char *message;
} Tram;

#endif