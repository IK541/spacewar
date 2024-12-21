all: client recv_server

client: client/client.cpp
	g++ -g client/client.cpp client/user_input.cpp client/draw.cpp -o target/client -lsfml-graphics -lsfml-window -lsfml-system -lsfml-network -lm -Wall

recv_server: server/recv_server.cpp
	g++ -g server/recv_server.cpp -o target/recv_server -Wall
