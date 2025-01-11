all: client server single-client single-server

client:
	g++ client/client.cpp -o target/client -Wall -Wextra -std=c++11 -pedantic -g

server: server/serv-threads.cpp
	g++ server/serv-threads.cpp server/Room.cpp server/Serv.cpp server/Player.cpp -o target/server -Wall -Wextra -std=c++11 -pedantic -g

single-client:
	g++ single-client/client.cpp single-client/out.cpp single-client/in.cpp -o target/single-client  -lsfml-graphics -lsfml-window -lsfml-system -lsfml-network -lm -Wall -Wextra -pedantic -std=c++11 -g

single-server:
	g++ single-server/server.cpp single-server/game_engine.cpp single-server/game_manager.cpp -o target/single-server  -lsfml-graphics -lsfml-window -lsfml-system -lsfml-network -lm -Wall -Wextra -pedantic -std=c++11 -g
