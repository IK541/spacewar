all: server

client_ge: client_ge/client.cpp
	g++ -g client_ge/client.cpp client_ge/user_input.cpp client_ge/draw.cpp -o target/client -lsfml-graphics -lsfml-window -lsfml-system -lsfml-network -lm -Wall

recv_server_ge: server_ge/recv_server.cpp
	g++ -g server_ge/recv_server.cpp -o target/recv_server -Wall

server: server/serv-threads.cpp
	g++ server/serv-threads.cpp server/Room.cpp server/Serv.cpp -o target/server -Wall -Wextra -std=c++11 -pedantic
