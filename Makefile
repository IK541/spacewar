all: client recv_server

client: client/client.cpp
	g++ client/client.cpp client/user_input.cpp -o target/client -lsfml-graphics -lsfml-window -lsfml-system -lsfml-network -lm -Wall

recv_server: server/recv_server.cpp
	g++ server/recv_server.cpp -o target/recv_server -Wall
