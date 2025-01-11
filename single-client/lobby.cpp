#include "lobby.hpp"

#define VIEW_SIZE 1000.0

void draw_name(NameState name_state, sf::RenderWindow* window, sf::Font* font, std::mutex* mtx) {
    std::lock_guard<std::mutex> lock(*mtx);
    window->setView(sf::View(sf::Vector2f(VIEW_SIZE,VIEW_SIZE), sf::Vector2f(2*VIEW_SIZE,2*VIEW_SIZE)));
    sf::RectangleShape name_field = sf::RectangleShape(sf::Vector2f(1.6*VIEW_SIZE,0.2*VIEW_SIZE));
    name_field.move(sf::Vector2f(0.2*VIEW_SIZE,0.9*VIEW_SIZE));
    name_field.setFillColor(sf::Color(63,63,63));
    
    sf::Text text;
    text.setFont(*font);
    text.setString(name_state.name.c_str());
    text.setCharacterSize(240);
    text.setFillColor(sf::Color::Yellow);
    text.move(0.21*VIEW_SIZE,0.85*VIEW_SIZE);

    window->clear();
    window->draw(name_field);
    window->draw(text);
    window->display();
}

void draw_lobby(LobbyState lobby_state, sf::RenderWindow* window, std::mutex* mtx) {
    std::lock_guard<std::mutex> lock(*mtx);
    window->setView(sf::View(sf::Vector2f(VIEW_SIZE,VIEW_SIZE), sf::Vector2f(2*VIEW_SIZE,2*VIEW_SIZE)));

    sf::RectangleShape selection = sf::RectangleShape(sf::Vector2f(1.6*VIEW_SIZE+0.2*VIEW_SIZE/ROOM_COUNT,1.8*VIEW_SIZE/ROOM_COUNT));
    selection.move(0.2*VIEW_SIZE-0.1*VIEW_SIZE/ROOM_COUNT,2*VIEW_SIZE/ROOM_COUNT*lobby_state.room_selected+0.1*VIEW_SIZE/ROOM_COUNT);
    selection.setFillColor(sf::Color::Yellow);

    sf::RectangleShape room_rects[ROOM_COUNT];
    std::vector<Triangle> ships;
    for(int i = 0; i < ROOM_COUNT; ++i) {
        room_rects[i].setSize(sf::Vector2f(1.6*VIEW_SIZE,1.6*VIEW_SIZE/ROOM_COUNT));
        room_rects[i].move(0.2*VIEW_SIZE,2*VIEW_SIZE/ROOM_COUNT*i+0.2*VIEW_SIZE/ROOM_COUNT);
        sf::Color color = lobby_state.rooms[i].is_game_running ? sf::Color(31,31,31) : sf::Color(63,63,63);
        room_rects[i].setFillColor(color);
        for(int j = 0; j < lobby_state.rooms[i].blue_count; ++j) {
            Triangle ship = Triangle(sf::Vector2f(
                VIEW_SIZE*(0.2+0.8/(PLAYERS_PER_TEAM+1)*(j+1)),
                2*VIEW_SIZE/ROOM_COUNT*i+VIEW_SIZE/ROOM_COUNT),
                VIEW_SIZE/10/PLAYERS_PER_TEAM,
                lobby_state.rooms[i].is_game_running ? sf::Color(0,0,127) : sf::Color(0,0,255), 0.0
            ); ships.push_back(ship);
        }
        for(int j = lobby_state.rooms[i].blue_count; j < PLAYERS_PER_TEAM; ++j) {
            Triangle ship = Triangle(sf::Vector2f(
                VIEW_SIZE*(0.2+0.8/(PLAYERS_PER_TEAM+1)*(j+1)),
                2*VIEW_SIZE/ROOM_COUNT*i+VIEW_SIZE/ROOM_COUNT),
                VIEW_SIZE/10/PLAYERS_PER_TEAM,
                lobby_state.rooms[i].is_game_running ? sf::Color(0,0,0) : sf::Color(31,31,31), 0.0
            ); ships.push_back(ship);
        }
        for(int j = 0; j < lobby_state.rooms[i].red_count; ++j) {
            Triangle ship = Triangle(sf::Vector2f(
                VIEW_SIZE*(1.8-0.8/(PLAYERS_PER_TEAM+1)*(j+1)),
                2*VIEW_SIZE/ROOM_COUNT*i+VIEW_SIZE/ROOM_COUNT),
                VIEW_SIZE/10/PLAYERS_PER_TEAM,
                lobby_state.rooms[i].is_game_running ? sf::Color(127,0,0) : sf::Color(255,0,0), M_PI
            ); ships.push_back(ship);
        }
        for(int j = lobby_state.rooms[i].red_count; j < PLAYERS_PER_TEAM; ++j) {
            Triangle ship = Triangle(sf::Vector2f(
                VIEW_SIZE*(1.8-0.8/(PLAYERS_PER_TEAM+1)*(j+1)),
                2*VIEW_SIZE/ROOM_COUNT*i+VIEW_SIZE/ROOM_COUNT),
                VIEW_SIZE/10/PLAYERS_PER_TEAM,
                lobby_state.rooms[i].is_game_running ? sf::Color(0,0,0) : sf::Color(31,31,31), M_PI
            ); ships.push_back(ship);
        }
    }

    window->clear();
    window->draw(selection);
    for(int i = 0; i < ROOM_COUNT; ++i) window->draw(room_rects[i]);
    for(Triangle ship: ships) ship.draw(window);
    window->display();
}

void draw_room(RoomState room_state, sf::RenderWindow* window, sf::Font* font, std::mutex* mtx) {
    std::lock_guard<std::mutex> lock(*mtx);
    window->setView(sf::View(sf::Vector2f(VIEW_SIZE,VIEW_SIZE), sf::Vector2f(2*VIEW_SIZE,2*VIEW_SIZE)));

    std::vector<sf::RectangleShape> blue;
    std::vector<sf::RectangleShape> red;
    std::vector<sf::CircleShape> blue_ready;
    std::vector<sf::CircleShape> red_ready;
    std::vector<sf::Text> blue_name;
    std::vector<sf::Text> red_name;
    int i = 0;
    for(PlayerInfo info: room_state.blue) {
        sf::RectangleShape background = sf::RectangleShape(sf::Vector2f(0.8*VIEW_SIZE,1.6*VIEW_SIZE/PLAYERS_PER_TEAM));
        background.move(0.1*VIEW_SIZE,2*VIEW_SIZE/PLAYERS_PER_TEAM*i+0.2*VIEW_SIZE/PLAYERS_PER_TEAM);
        background.setFillColor(sf::Color(31,31,31));
        blue.push_back(background);
        if(info.ready) {
            sf::CircleShape ready = sf::CircleShape(0.1*VIEW_SIZE/PLAYERS_PER_TEAM);
            ready.move(0.1*VIEW_SIZE+0.1*VIEW_SIZE/PLAYERS_PER_TEAM,2*VIEW_SIZE/PLAYERS_PER_TEAM*i+0.9*VIEW_SIZE/PLAYERS_PER_TEAM);
            ready.setFillColor(sf::Color::Yellow);
            blue_ready.push_back(ready);
        }
        sf::Text name = sf::Text(info.name, *font, 120);
        name.setFillColor(sf::Color(0,0,255));
        name.move(0.1*VIEW_SIZE+0.4*VIEW_SIZE/PLAYERS_PER_TEAM,2*VIEW_SIZE/PLAYERS_PER_TEAM*i+0.775*VIEW_SIZE/PLAYERS_PER_TEAM);
        blue_name.push_back(name);
        ++i;
    }
    i = 0;
    for(PlayerInfo info: room_state.red) {
        sf::RectangleShape background = sf::RectangleShape(sf::Vector2f(0.8*VIEW_SIZE,1.6*VIEW_SIZE/PLAYERS_PER_TEAM));
        background.move(1.1*VIEW_SIZE,2*VIEW_SIZE/PLAYERS_PER_TEAM*i+0.2*VIEW_SIZE/PLAYERS_PER_TEAM);
        background.setFillColor(sf::Color(31,31,31));
        blue.push_back(background);
        if(info.ready) {
            sf::CircleShape ready = sf::CircleShape(0.1*VIEW_SIZE/PLAYERS_PER_TEAM);
            ready.move(1.1*VIEW_SIZE+0.1*VIEW_SIZE/PLAYERS_PER_TEAM,2*VIEW_SIZE/PLAYERS_PER_TEAM*i+0.9*VIEW_SIZE/PLAYERS_PER_TEAM);
            ready.setFillColor(sf::Color::Yellow);
            red_ready.push_back(ready);
        }
        sf::Text name = sf::Text(info.name, *font, 120);
        name.setFillColor(sf::Color(255,0,0));
        name.move(1.1*VIEW_SIZE+0.4*VIEW_SIZE/PLAYERS_PER_TEAM,2*VIEW_SIZE/PLAYERS_PER_TEAM*i+0.775*VIEW_SIZE/PLAYERS_PER_TEAM);
        red_name.push_back(name);
        ++i;
    }

    window->clear();
    for(sf::RectangleShape background: blue) window->draw(background);
    for(sf::RectangleShape background: red) window->draw(background);
    for(sf::CircleShape ready: blue_ready) window->draw(ready);
    for(sf::CircleShape ready: red_ready) window->draw(ready);
    for(sf::Text name: blue_name) window->draw(name);
    for(sf::Text name: red_name) window->draw(name);
    window->display();
}
