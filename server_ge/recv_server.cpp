#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdio.h>

#define LISTEN_QUEUE 1
#define BUFFER_SIZE 4096

#define SERVER_PORT 10000

int main() {
    int sfd = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if(sfd < 0) perror("socket error");
    int one = 1;
    setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one));
    sockaddr_in saddr;
    saddr.sin_family = AF_INET;
    saddr.sin_addr.s_addr = htonl(INADDR_ANY);
    saddr.sin_port = htons(SERVER_PORT);
    if(bind(sfd, (sockaddr*) &saddr, sizeof(saddr))) perror("bind error");
    sockaddr_in caddr;
    socklen_t caddr_size = sizeof(caddr);

    int size = 0;
    char buffer[BUFFER_SIZE];
    while (1) {
        size = recvfrom(sfd, buffer, BUFFER_SIZE, 0, (sockaddr*) &caddr, &caddr_size);
        if(size < 9) continue;
        printf("{%s : %d} ", inet_ntoa(caddr.sin_addr), ntohs(caddr.sin_port)); fflush(stdout);
        uint32_t timestamp = *(unsigned*)buffer;
        float direction = *(float*)(buffer+4);
        uint8_t shooting = *(buffer+8) & 0x02;
        uint8_t engine_on = *(buffer+8) & 0x01;
        printf("%d %f %d %d\n", timestamp, direction, shooting, engine_on);
    }
    close(sfd);
    return 0;
}
