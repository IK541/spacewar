#include "client.hpp"

int main(int argc, char **argv) {
    int sock = getaddrinfo_socket_connect(argc, argv);

    char data[10];
    int sent = 1;
    while (true) {
        write(sock, data, sizeof(data));
        printf("   Sent %d\r", 10 * sent++);
        fflush(stdout);
    }
}
