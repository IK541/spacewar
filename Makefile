all: client server

client: client.cpp
	g++ client.cpp -o client -lsfml-graphics -lsfml-window -lsfml-system -lsfml-network -lm -Wall

server: server.cpp
	g++ server.cpp -o server -Wall
