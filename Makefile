all: client server

client:
	g++ -g client/client.cpp client/user_input.cpp client/draw.cpp -o target/client  -lsfml-graphics -lsfml-window -lsfml-system -lsfml-network -lm -Wall

server:
	g++ -g server/server.cpp server/game_engine.cpp server/room_manager.cpp -o target/server  -lsfml-graphics -lsfml-window -lsfml-system -lsfml-network -lm -Wall
