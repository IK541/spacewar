all: client server

client:
	g++ single-client/client.cpp single-client/out.cpp single-client/in.cpp -o target/single-client  -lsfml-graphics -lsfml-window -lsfml-system -lsfml-network -lm -Wall -Wextra -pedantic -std=c++11 -g

server:
	g++ single-server/server.cpp single-server/game_engine.cpp single-server/game_manager.cpp -o target/single-server  -lsfml-graphics -lsfml-window -lsfml-system -lsfml-network -lm -Wall -Wextra -pedantic -std=c++11 -g
