#include <cstdio>
#include <cstdlib>
#include <errno.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <thread>
#include <arpa/inet.h>
#include "serv.hpp"
#include <iostream>


void handleClient(Server server, int sock){
    server.handleClient(sock);
}

int main(){


    Server server;


    while (true) {
        sockaddr_in clientAddr;
        socklen_t clientAddrSize = sizeof(clientAddr);
        int clientSocket = accept(server.servSock, (sockaddr*)&clientAddr , &clientAddrSize);
        if (clientSocket == -1){
            perror("Blad podczas skceptowania polaczenia");
            continue;
        }
        //std::cout << "Nowe polaczenie od " << inet_ntoa(clientAddr.sin_addr) << "\n";

        std::thread clientThread(handleClient, server, clientSocket);
        clientThread.detach();

        


    }

    return 0;
}