#include "connection.h"

void hello_world();
void send_file();

int main() {
    //hello_world();
    send_file(); 

    return 0;
}


void hello_world(){
    broadcast_send_message("hello", "10.1.1.3", 1900);
}

void send_file(){
    broadcast_send_file("config/R1/routing_table.yaml", "10.1.1.3", 1900);
}