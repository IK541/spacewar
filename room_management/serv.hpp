#pragma once
#include <cstdio>
#include <cstdlib>
#include <errno.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <vector>
#include "room.hpp"
#include <iostream>
#include <cstring>

class Server{
    public:
    int servSock;
    sockaddr_in localAddress;
    const int MaxPlayers = 6;
    const int PORT = 3055;
    std::vector<Room> rooms;




    
    Server(){

    sockaddr_in address{.sin_family = AF_INET, .sin_port = htons(PORT), .sin_addr = {htonl(INADDR_ANY)}};
    localAddress = address;
    int servSock = socket(PF_INET, SOCK_STREAM, 0);

    const int one = 1;
    setsockopt(servSock, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one));

    if (bind(servSock, (sockaddr*)&localAddress, sizeof(localAddress))){
        perror("Bind failed");
        exit(1);
    }

    listen(servSock, 1);
    std::cout << "Serwer uruchomiony na porcie " << PORT << "Oczekiweanie na graczy...\n";
    
    Room r1;
    Room r2;
    rooms.push_back(r1);
    rooms.push_back(r2);
    }

  

    void handleClient(int clientSocket){
        char buffer[256];
        memset(buffer, 0, sizeof(buffer));
        //nick
        
        // char *prompt = "give_nick";
        // send(clientSocket, prompt, strlen(prompt), 0);

        // pobierz nick
        int msglen = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
        if (msglen <= 0){
            perror("blad uzyskiwania nicku klienta");
            close(clientSocket);
            return ;
        }
        
        std::string nick(buffer);
        nick.erase(nick.find_last_not_of("\n\r") + 1); //usuwa znak nowej linii
        
        
        Player p (nick, clientSocket);



        memset(buffer, 0, sizeof(buffer));
        //room
        std::string msg = "choose room\n";
        msg += std::to_string(rooms[0].get_number_of_players());
        send(clientSocket, msg.c_str(), strlen(msg.c_str()), 0);

        
        msglen = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
        if (msglen <= 0){
            perror("blad uzyskiwania pokoju");
            close(clientSocket);
            return ;
        }
        //std::cout << "Gracz " << p.name << "dolaczyl do pokoju " << buffer << "\n";


       


    }


};



