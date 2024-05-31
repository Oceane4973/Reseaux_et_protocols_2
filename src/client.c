#include "connection.h"

void hello_world();

int main() {
    hello_world();

    return 0;
}

void hello_world(){
    char* message = "Hello, World!";
    char* ip = "10.1.6.2";
    int port = 8520;

    send_message_on_standard_socket(message, ip, port);
}
