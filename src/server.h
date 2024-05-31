#ifndef SERVER_H
#define SERVER_H

typedef struct {
    char *name;
    char *ip;
    int port;
    int mask;
} Server;


void destroyServer(Server *server);
void *start_server(void *Server);

#endif