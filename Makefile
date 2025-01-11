all: server single-client single-server

client_ge: client_ge/client.cpp
	g++ -g client_ge/client.cpp client_ge/user_input.cpp client_ge/draw.cpp -o target/client -lsfml-graphics -lsfml-window -lsfml-system -lsfml-network -lm -Wall

recv_server_ge: server_ge/recv_server.cpp
	g++ -g server_ge/recv_server.cpp -o target/recv_server -Wall

server: server/serv-threads.cpp
	g++ server/serv-threads.cpp server/Room.cpp server/Serv.cpp -o target/server -Wall -Wextra -std=c++11 -pedantic

single-client:
	g++ single-client/client.cpp single-client/out.cpp single-client/in.cpp -o target/single-client  -lsfml-graphics -lsfml-window -lsfml-system -lsfml-network -lm -Wall -Wextra -pedantic -std=c++11 -g

single-server:
	g++ single-server/server.cpp single-server/game_engine.cpp single-server/game_manager.cpp -o target/single-server  -lsfml-graphics -lsfml-window -lsfml-system -lsfml-network -lm -Wall -Wextra -pedantic -std=c++11 -g
