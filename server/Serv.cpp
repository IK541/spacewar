#include "Serv.hpp"
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

#include <queue>
#include <string>
#define BUFFER_SIZE 256


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

        pfds[MAX_CLIENTS].fd = server_fd;
        pfds[MAX_CLIENTS].events = POLLIN;

        std::cout << "server created and binded successfully\n";
}

void Serv::serve(){
    // Start listening
    if (listen(server_fd, 10) < 0) {
        perror("Listen failed");
        exit(-1);
    }


    std::cout << "Server is listening on port " << port << "...\n";
    while (work) {
        int num_events = poll(pfds, MAX_CLIENTS + 1, -1);
        if (num_events < 0) {
            perror("Poll failed");
            break;
        }

        // Check if there's a new connection
        if (pfds[MAX_CLIENTS].revents & POLLIN) {
            handle_new_connection();
            --num_events; // One event handled
        }

        // Handle events for existing clients
        for (int i = 0; i < MAX_CLIENTS && num_events > 0; ++i) {
            if (pfds[i].fd == -1) continue; // Skip unused slots

            // Check if there's data to read
            if (pfds[i].revents & POLLIN) {
                handle_client_input(i);
                --num_events;
            }

            // Check if ready to write (optional, if implementing non-blocking send)
            if (pfds[i].revents & POLLOUT) {
                handle_client_output(i);
                --num_events;
            }

            // Check for error conditions
            if (pfds[i].revents & (POLLERR | POLLHUP | POLLNVAL)) {
                disconnect_client(i);
                --num_events;
            }
        }
    }

    close(server_fd);
    return;
}


void Serv::handle_new_connection() {
    sockaddr_in player_address;
    socklen_t addr_len = sizeof(player_address);
    int client_fd = accept(server_fd, (struct sockaddr *)&player_address, &addr_len);
    if (client_fd < 0) {
        perror("Accept failed");
        return;
    }

    // Find a free slot
    for (int i = 0; i < MAX_CLIENTS; ++i) {
        if (!free_pfds[i]) {
            free_pfds[i] = true;
            pfds[i].fd = client_fd;
            pfds[i].events = POLLIN;
            std::cout << "Client connected on slot " << i << "\n";

            Player::players[i].take(player_address, client_fd);
            return;
        }
    }

    // Server full
    std::string msg = "Server is full, try again later\n";
    cout << "denied client, too much users\n";
    send(client_fd, msg.c_str(), msg.size(), 0);
    shutdown(client_fd, SHUT_RDWR);
    close(client_fd);
}

void Serv::handle_client_input(int client_id) {
    char buffer[1024];
    string msg = "";
    string nick = "";
    int bytes_read = recv(pfds[client_id].fd, buffer, sizeof(buffer) - 1, 0);

    if (bytes_read <= 0) {
        if (bytes_read == 0) {
            std::cout << "Client " << client_id << " disconnected.\n";
        } else {
            perror("Recv failed");
        }
        disconnect_client(client_id);
        return;
    }

    buffer[bytes_read] = '\0'; // Null-terminate the message
    std::cout << "Received from client " << client_id << ": " << buffer << "\n";

    // Check if there's a message
    if (bytes_read > 0) {
        char first_char = buffer[0]; // First letter of the message
        int room_char;

        switch (first_char) {
            case 'A':
            case 'a':
            
                {
                nick = string(buffer).substr(2, bytes_read);
                std::cout << "Client " << client_id << " sent an 'A'-type message: nick.\n";
                // receive nick from player
                lock_guard<std::mutex> lock(mtx);

                if(Player::players[client_id].set_nick(nick)){
                    vector<char> binary_lobby = Room::get_binary_general_room_info();
                    char bf[13];
                    for (int i = 0; i <= 13; i++)
                        bf[i] = binary_lobby[i];

                    send_to_player(client_id, bf, 13);
                }
                else
                    send_to_player(client_id, "N");
                
                }
                break;
                

            case 'B':
            case 'b':

            
                std::cout << "Client " << client_id << " sent a 'B'-type message: get room info.\n";
                // send all rooms info
                msg = "";
                
                msg += Room::get_general_room_info();
                
                send_to_player(client_id, msg);
                cv.notify_one();
                break;

            // case 'C': // Send room details
            // case 'c':
                


            //     std::cout << "Client " << client_id << " sent a 'C'-type message: get specific room info.\n";
            //     if (buffer[2] < '0' && buffer[2] >'3') msg = "N\n";
            //     else{
            //         room_char = buffer[2] - '0';
            //         msg += Room::rooms[room_char].get_room_info();

            //     }
            //     send_to_player(client_id, msg);
            //     cv.notify_one();
            //     break;
            // human like
            case 'Z': // Send room details
            case 'z':
                


                std::cout << "Client " << client_id << " sent a 'Z'-type message: get specific room info.\n";
                if (buffer[2] < '0' && buffer[2] >'3') msg = "N\n";
                else{
                    room_char = buffer[2] - '0';
                    msg += Room::rooms[room_char].get_room_info_human();

                }
                send_to_player(client_id, msg);
                cv.notify_one();
                break;

            case 'D': // enter room n
            case 'd':
                {
                lock_guard<std::mutex> lock(mtx);
                std::cout << "Client " << client_id << " sent a 'D'-type message: enter room.\n";
                if (buffer[2] < '0' && buffer[2] >'3') msg = "N";
                else{
                    room_char = buffer[2] - '0';


                    msg = Room::rooms[room_char].join_room(client_id);
                    if(msg[0] == 'Y')
                        events.push("0" + std::to_string(room_char));

                }
                send_to_player(client_id, msg);
                cv.notify_one();
                }
                break;
                

            case 'E':
            case 'e': // switch teams
                {
                lock_guard<std::mutex> lock(mtx);
                std::cout << "Client " << client_id << " sent a 'E'-type message: switch team.\n";
                
                msg = Room::rooms[Player::players[client_id].room].switch_teams(client_id);
                
                if(msg[0] == 'Y')
                        events.push("0" + std::to_string(Player::players[client_id].room));

                send_to_player(client_id, msg);
                cv.notify_one();
                }
                break;

            case 'F':
            case 'f':
                {
                lock_guard<std::mutex> lock(mtx);
                std::cout << "Client " << client_id << " sent a 'F'-type message: change ready state.\n";

                msg = Player::players[client_id].change_ready_state();
                if(msg[0] == 'Y')
                        events.push("1" + std::to_string(Player::players[client_id].room));// FAILS HERE
                else
                    send_to_player(client_id, "N");
                cv.notify_one();
                }
                break;

            default:
                std::cout << "Client " << client_id << " sent an unrecognized message type: " << first_char << "\n";
                send(pfds[client_id].fd, "Unrecognized command\n", 21, 0);
                break;
        }
    }
}

void Serv::disconnect_client(int client_id) {
    lock_guard<std::mutex> lock(mtx);
    if (Player::players[client_id].room != -1)
        events.push("0" + std::to_string(Player::players[client_id].room));
    cv.notify_one();

    if(pfds[client_id].fd > 0)
        close(pfds[client_id].fd);
    pfds[client_id].fd = -1;
    pfds[client_id].events = 0;
    free_pfds[client_id] = false;

    if (Player::players[client_id].room != -1) {
        int room_id = Player::players[client_id].room;
        Room::rooms[Player::players[client_id].room].teams_player_number[Player::players[client_id].team]--;

        Room::rooms[room_id].free_slots++;
    }
    Player::players[client_id].make_free();
    std::cout << "Client " << client_id << " has been disconnected.\n";
}

void Serv::handle_client_output(int client_id) {
    // Example: Send a queued message
    std::string msg = "Server new night response\n";
    send(pfds[client_id].fd, msg.c_str(), msg.size(), 0);
}


void Serv::cleanup(){

    lock_guard<std::mutex> server_lock(Serv::mtx);
    lock_guard<std::mutex> room_lock(Room::rooms_mutex);
    for(int i = 0; i < Player::max_players; ++i)
        Player::players[i].mtx.lock();
    shutdown(server_fd, SHUT_RDWR);
    close(server_fd);

    for(int i = Player::max_players; i >= 0; i--) {
        Player::players[i].make_free();
    }
    for(int i = Player::max_players; i >= 0; i--){
        if (pfds[i].fd == -1) continue;
        shutdown(pfds[i].fd, SHUT_RDWR);
        close(pfds[i].fd);
    }

        // for(int i = 0; i < Player::max_players; i++){
    //     if(Player::players[i].fd != -1) {        
    //         shutdown(Player::players[i].fd, SHUT_RDWR);
    //         close(Player::players[i].fd);
    //     }
    // }
    for(int i = 0; i < Player::max_players; ++i)
        Player::players[i].mtx.unlock();

}



void Serv::monitor(){
    while (true) {
        unique_lock<mutex> lock(mtx);
        cv.wait(lock, [this] { return !events.empty() || stop; });

        while (!events.empty()) {
            string msg = "";
            vector<char> binary_lobby;
                        // events type:= 0: detailed, 1: general
            string event = events.front();
            cout << "detected event: " << event << endl;
            if(event[0] == '0'){ // detailed room info update
                binary_lobby = Room::rooms[stoi(event.substr(1))].get_room_info();
                int len = binary_lobby.size();
                char msg_bin_room[len];
                for(unsigned int i = 0; i < binary_lobby.size(); i++){
                    msg_bin_room[i] = binary_lobby[i];
                }
                send_to_room_members(stoi(event.substr(1)), msg_bin_room, binary_lobby.size());
            }


            char msg_bin[13];
            binary_lobby = Room::get_binary_general_room_info();
            for(int i = 0; i < 14; i++){
                msg_bin[i] = binary_lobby[i];
            }
            send_to_lobby_members(msg_bin, 13);

            events.pop();
        }

        if (stop && events.empty()) {
            break;
        }
    }

}


void Serv::send_to_room_members(int room_id, string msg){
    for(int i = 0; i < Player::max_players; i++){
        if(Player::players[i].room == room_id)
            send_to_player(i, msg);
    }
};

void Serv::send_to_room_members(int room_id, char* msg, int len){
    for(int i = 0; i < Player::max_players; i++){
        if(Player::players[i].room == room_id)
            send_to_player(i, msg, len);
    }
};

void Serv::send_to_lobby_members(string msg){
    
    for(int i = 0; i < Player::max_players; i++){
        if(Player::players[i].room == -1 && Player::players[i].fd != -1)
            send_to_player(i, msg);
    }

};

void Serv::send_to_lobby_members(char* msg, int len){
    
    for(int i = 0; i < Player::max_players; i++){
        if(Player::players[i].room == -1 && Player::players[i].fd != -1)
            send_to_player(i, msg, len);
    }

};

void Serv::send_to_player(int player_id, string msg){
    lock_guard<std::mutex> lock(Player::players[player_id].mtx); // PLAYER MUTEX 
    send(pfds[player_id].fd, msg.c_str(), msg.size(), 0);
}

void Serv::send_to_player(int player_id, char* msg, int len){
    lock_guard<std::mutex> lock(Player::players[player_id].mtx); // PLAYER MUTEX 
    send(pfds[player_id].fd, msg, len, 0);
}