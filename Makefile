all: client recv_server single

client: client/client.cpp
	g++ -g client/client.cpp client/user_input.cpp client/draw.cpp -o target/client -lsfml-graphics -lsfml-window -lsfml-system -lsfml-network -lm -Wall

recv_server: server/recv_server.cpp
	g++ -g server/recv_server.cpp -o target/recv_server -Wall

single:
	g++ -g client/single.cpp client/user_input.cpp client/draw.cpp server/game_engine.cpp -o target/single  -lsfml-graphics -lsfml-window -lsfml-system -lsfml-network -lm -Wall
