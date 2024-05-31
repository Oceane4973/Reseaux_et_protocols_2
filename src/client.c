#include "connection.h"

void hello_world();
void send_tram();

int main() {
    send_tram();
    //hello_world();

    return 0;
}

void hello_world(){
    char* message = "Hello, World!";
    char* ip = "10.1.6.2";
    int port = 8520;

    send_message_on_standard_socket(message, sizeof(message), ip, port);
}

void send_tram() {
    Tram *tram = (Tram *)malloc(sizeof(Tram));
    if (!tram) {
        perror("Failed to allocate memory for Tram");
        exit(EXIT_FAILURE);
    }

    // Initialiser les champs de la structure Tram
    tram->destination = "Destination";
    tram->origin = "Origin";
    tram->port = 8080;
    tram->message = "Hello";

    // Convertir la structure Tram en buffer
    char* message = tram_to_buffer(tram);

    char *ip = "10.1.6.2";
    int port = 8520;

    // Envoyer le message sur le socket en utilisant la taille correcte du buffer
    send_message_on_standard_socket(message, sizeof(message),ip, port);
}
