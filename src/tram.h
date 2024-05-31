#ifndef TRAM_H
#define TRAM_H

typedef struct {
    char *destination;
    char *origin;
    int port;
    char *message;
} Tram;

#define MAX_BUFFER_SIZE 1024

void destroyTram(Tram *tram);
char* tram_to_buffer(const Tram *tram);
Tram* buffer_to_tram(const char *buffer);
char* display_tram(Tram *tram);

#endif