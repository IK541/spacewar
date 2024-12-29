all: client server

client:
	g++ client/client.cpp client/out.cpp client/in.cpp -o target/client  -lsfml-graphics -lsfml-window -lsfml-system -lsfml-network -lm -Wall -Wextra -pedantic -std=c++20 -g

server:
	g++ server/server.cpp server/game_engine.cpp server/room_manager.cpp -o target/server  -lsfml-graphics -lsfml-window -lsfml-system -lsfml-network -lm -Wall -Wextra -pedantic -std=c++20 -g
