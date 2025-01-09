#include "Serv.hpp"

#include <cstring>
#include <sys/socket.h>
#include <poll.h>
#include <netinet/in.h>
#include <unistd.h>
#include <csignal>
#include <iostream>
#include <thread>
#include <mutex>
#include "Room.hpp"

#define MAX_EVENTS 8
#define MAX_CLIENTS 20 // >= ROOM_COUNT * PLAYERS_PER_TEAM * 2

using namespace std;




Serv::Serv(int _port){
        port = _port;
        int opt = 1;
        addrlen = sizeof(address);

        if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
            perror("Socket failed");
            exit(-1);
        }

        // Attach socket to the port
        if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
            perror("Setsockopt failed");
            exit(-1);
        }

        address.sin_family = AF_INET;
        address.sin_addr.s_addr = INADDR_ANY;
        address.sin_port = htons(port);

        if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
            perror("Bind failed");
            exit(-1);
        }

        std::cout << "server created and binded successfully\n";


}


void Serv::serve(Player players[], Room rooms[], mutex *mtx){

        // Start listening
        if (listen(this->server_fd, 10) < 0) {
            perror("Listen failed");
            exit(-1);
        }

        pollfd pfds[MAX_CLIENTS+1]{};
        bool free_pfds[MAX_CLIENTS]{}; // TODO: move this to global client table
        pfds[MAX_CLIENTS].fd = server_fd;
        pfds[MAX_CLIENTS].events = POLLIN;

        std::cout << "Server is listening on port " << port << "...\n";
        while (true) {
            poll(pfds, MAX_CLIENTS + 1, -1);
            // accepter
            if(pfds[MAX_CLIENTS].revents & POLL_IN) {
                int free_cid = 0;
                sockaddr_in player_address;
                int client_fd = accept(server_fd, (struct sockaddr *)&player_address, (socklen_t*)&player_address);
                if (client_fd < 0) perror("Accept failed");
                else {
                    std::cout << "New client connected.\n";
                    bool success = false;
                    mtx->lock();
                    for(int cid = 0; cid < MAX_CLIENTS; ++cid) {
                        if(!free_pfds[cid]) continue;
                        free_pfds[cid] = false; // TODO: remember to zero this at disconnect
                        success = true;
                        pfds[cid].fd = client_fd;
                        pfds[cid].events = POLL_IN; // TODO: remeber to zero this field once client is disconnected
                        free_cid = cid;
                        
                        // TODO: save client address here - will be useful for UDP




                        break;
                    } if(!success) {
                        mtx->unlock();
                        std::string msg = "server is full, try again later";
                        send(client_fd, msg.c_str(), strlen(msg.c_str()), 0);
                        shutdown(client_fd, SHUT_RDWR);
                        close(client_fd);
                    }
                    
                    Player p;
                    p.take(player_address, client_fd);
                    players[free_cid] = p;
                    mtx->unlock();
                    // creating and saving client


                }
            }
            // client handler
            


            for(int cid = 0; cid < MAX_CLIENTS; ++cid) {
                if(free_pfds[cid] || !(pfds[cid].revents & POLL_IN)) continue;
                
            }
        }

        close(server_fd);
        return;

    }



void Serv::handle_client(int client_fd){
    char buffer[BUFFER_SIZE] = {0};

    memset(buffer, 0, BUFFER_SIZE);

    // get nickname
    std::string nickname;
    std::string msg = "give nick";

    send(client_fd, msg.c_str(), strlen(msg.c_str()), 0);

    int bytes_read = read(client_fd, buffer, BUFFER_SIZE);
        if (bytes_read <= 0) {
            std::cout << "nick error disconnected.\n";
        }

    nickname = std::string(buffer, bytes_read - 1);
    memset(buffer, 0, BUFFER_SIZE);

    std::cout << "nick got:" << nickname << "\n";

    //give rooms
    std::string rooms = "room1: 2, 0\nroom2: 0,0";
    send(client_fd, rooms.c_str(), strlen(rooms.c_str()), 0);

    
    while (true) {
        memset(buffer, 0, BUFFER_SIZE);
        int bytes_read = read(client_fd, buffer, BUFFER_SIZE);
        if (bytes_read <= 0) {
            std::cout << "Client disconnected.\n";
            break;
        }
        std::cout << "Received from client: " << buffer;
        send(client_fd, buffer, bytes_read, 0);
    }

    close(client_fd);
}