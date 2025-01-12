all: server client single-server

server: server/serv-threads.cpp
	g++ server/serv-threads.cpp server/Room.cpp server/Serv.cpp server/Player.cpp single-server/game_engine.cpp single-server/game_manager.cpp -o target/server -Wall -Wextra -pedantic -std=c++11 -g

client:
	g++ client/client.cpp client/game_out.cpp client/game_in.cpp client/lobby.cpp client/common.cpp -o target/client  -lsfml-graphics -lsfml-window -lsfml-system -lsfml-network -lm -Wall -Wextra -pedantic -std=c++11 -g

single-server:
	g++ single-server/server.cpp single-server/game_engine.cpp single-server/game_manager.cpp -o target/single-server  -lsfml-graphics -lsfml-window -lsfml-system -lsfml-network -lm -Wall -Wextra -pedantic -std=c++11 -g
