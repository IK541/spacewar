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
#include "Player.hpp"

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

void Serv::serve(mutex *mtx){
    // Start listening
    if (listen(server_fd, 10) < 0) {
        perror("Listen failed");
        exit(-1);
    }

    pollfd pfds[MAX_CLIENTS + 1]{};

    bool free_pfds[MAX_CLIENTS]{};
    pfds[MAX_CLIENTS].fd = server_fd;
    pfds[MAX_CLIENTS].events = POLLIN;

    std::cout << "Server is listening on port " << port << "...\n";
    while (true) {
        int num_events = poll(pfds, MAX_CLIENTS + 1, -1);
        if (num_events < 0) {
            perror("Poll failed");
            break;
        }

        // Check if there's a new connection
        if (pfds[MAX_CLIENTS].revents & POLLIN) {
            handle_new_connection(pfds, free_pfds, server_fd);
            --num_events; // One event handled
        }

        // Handle events for existing clients
        for (int i = 0; i < MAX_CLIENTS && num_events > 0; ++i) {
            if (pfds[i].fd == -1) continue; // Skip unused slots

            // Check if there's data to read
            if (pfds[i].revents & POLLIN) {
                handle_client_input(i, pfds, free_pfds);
                --num_events;
            }

            // Check if ready to write (optional, if implementing non-blocking send)
            if (pfds[i].revents & POLLOUT) {
                handle_client_output(i, pfds, free_pfds);
                --num_events;
            }

            // Check for error conditions
            if (pfds[i].revents & (POLLERR | POLLHUP | POLLNVAL)) {
                disconnect_client(i, pfds, free_pfds);
                --num_events;
            }
        }
    }

    close(server_fd);
    return;
}

void Serv::serve(Player players[], Room rooms[], mutex *mtx){

    // Start listening
    if (listen(this->server_fd, 10) < 0) {
        perror("Listen failed");
        exit(-1);
    }

    pollfd pfds[20+1]{};

    bool free_pfds[MAX_CLIENTS]{}; // TODO: move this to global client table
    pfds[MAX_CLIENTS].fd = server_fd;
    pfds[MAX_CLIENTS].events = POLLIN;

    std::cout << "Server is listening on port " << port << "...\n";
    while (true) {
        int num_events = poll(pfds, MAX_CLIENTS + 1, -1);
        if (num_events < 0) {
            perror("Poll failed");
            break;
        }

        // Check if there's a new connection
        if (pfds[MAX_CLIENTS].revents & POLLIN) {
            handle_new_connection(pfds, free_pfds, server_fd);
            --num_events; // One event handled
        }

        // Handle events for existing clients
        for (int i = 0; i < MAX_CLIENTS && num_events > 0; ++i) {
            if (pfds[i].fd == -1) continue; // Skip unused slots

            // Check if there's data to read
            if (pfds[i].revents & POLLIN) {
                handle_client_input(i, pfds, free_pfds);
                --num_events;
            }

            // Check if ready to write (optional, if implementing non-blocking send)
            if (pfds[i].revents & POLLOUT) {
                handle_client_output(i, pfds, free_pfds);
                --num_events;
            }

            // Check for error conditions
            if (pfds[i].revents & (POLLERR | POLLHUP | POLLNVAL)) {
                disconnect_client(i, pfds, free_pfds);
                --num_events;
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

void Serv::handle_new_connection(pollfd pfds[], bool free_pfds[], int server_fd) {
    sockaddr_in player_address;
    socklen_t addr_len = sizeof(player_address);
    int client_fd = accept(server_fd, (struct sockaddr *)&player_address, &addr_len);
    if (client_fd < 0) {
        perror("Accept failed");
        return;
    }

    // Find a free slot
    serv_mutex.lock();
    for (int i = 0; i < MAX_CLIENTS; ++i) {
        if (!free_pfds[i]) {
            free_pfds[i] = true;
            pfds[i].fd = client_fd;
            pfds[i].events = POLLIN;
            std::cout << "Client connected on slot " << i << "\n";

            Player::players[i].take(player_address, client_fd);
            serv_mutex.unlock();
            return;
        }
    }

    // Server full
    std::string msg = "Server is full, try again later\n";
    serv_mutex.unlock();
    cout << "denied client, too much users\n";
    send(client_fd, msg.c_str(), msg.size(), 0);
    shutdown(client_fd, SHUT_RDWR);
    close(client_fd);
}


void Serv::handle_client_input(int client_id, pollfd *pfds, bool *free_pfds) {
    char buffer[1024];
    string msg = "";
    int bytes_read = recv(pfds[client_id].fd, buffer, sizeof(buffer) - 1, 0);

    if (bytes_read <= 0) {
        if (bytes_read == 0) {
            std::cout << "Client " << client_id << " disconnected.\n";
        } else {
            perror("Recv failed");
        }
        disconnect_client(client_id, pfds, free_pfds);
        return;
    }

    buffer[bytes_read] = '\0'; // Null-terminate the message
    std::cout << "Received from client " << client_id << ": " << buffer << "\n";

    // Check if there's a message
    if (bytes_read > 0) {
        char first_char = buffer[0]; // First letter of the message

        switch (first_char) {
            case 'A':
            case 'a':
                std::cout << "Client " << client_id << " sent an 'A'-type message: nick.\n";
                // Handle 'A' type message
                Player::players[client_id].setNick(buffer + 1);
                send(pfds[client_id].fd, "Acknowledged A\n", 14, 0);
                break;

            case 'B':
            case 'b':
                std::cout << "Client " << client_id << " sent a 'B'-type message: get room info.\n";
                // Handle 'B' type message
                msg = Room::getGeneralRoomInfo();
                
                send(pfds[client_id].fd, msg.c_str(), msg.size(), 0);
                break;

            case 'C':
            case 'c':
                std::cout << "Client " << client_id << " sent a 'C'-type message: choosed room.\n";
                // Handle 'C' type message
                // TODO implement room choosing
                // Send room details to player
                send(pfds[client_id].fd, "Acknowledged C\n", 14, 0);
                break;
            // TODO change ready state
            // TODO change game
            default:
                std::cout << "Client " << client_id << " sent an unrecognized message type: " << first_char << "\n";
                send(pfds[client_id].fd, "Unrecognized command\n", 21, 0);
                break;
        }
    }
}



void Serv::disconnect_client(int client_id, pollfd *pfds, bool *free_pfds) {
    close(pfds[client_id].fd);
    pfds[client_id].fd = -1;
    pfds[client_id].events = 0;
    free_pfds[client_id] = false;

    if (Player::players[client_id].room != -1) {
        int room_id = Player::players[client_id].room;
        Room::rooms[room_id].remove_player(client_id);
    }
    Player::players[client_id].make_free();
    std::cout << "Client " << client_id << " has been disconnected.\n";
}

void Serv::handle_client_output(int client_id, pollfd *pfds, bool *free_pfds) {
    // Example: Send a queued message
    std::string msg = "Server new night response\n";
    send(pfds[client_id].fd, msg.c_str(), msg.size(), 0);
}


