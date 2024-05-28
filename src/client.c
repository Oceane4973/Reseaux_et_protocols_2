#include "connection.h"


int main() {
    broadcast_send_message("hello", "10.1.1.3", 1900);

    return 0;
}
