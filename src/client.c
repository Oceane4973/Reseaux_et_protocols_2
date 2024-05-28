#include "connection.h"


int main() {
    broadcast_send_message("hello", "192.1.1.255", 1900);

    return 0;
}
