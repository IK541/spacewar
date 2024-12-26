#include "draw.hpp"
#include <cmath>
#include "cstring"

Shape::Shape(sf::Vector2f pos, float scale, float angle, sf::Color color):
pos(pos),scale(scale),angle(angle),color(color){}

Triangle::Triangle(sf::Vector2f pos, float scale, float angle, sf::Color color):
Shape(pos,scale,angle,color){}

Circle::Circle(sf::Vector2f pos, float scale, sf::Color color):
Shape(pos,scale,0.0,color){}

void Circle::draw(sf::RenderWindow* window) {
    sf::CircleShape shape(0.5*this->scale);
    shape.setOrigin(0.5*this->scale, 0.5*this->scale);
    shape.move(this->pos);
    shape.setFillColor(this->color);
    window->draw(shape);
}

void Triangle::draw(sf::RenderWindow* window) {
    // TODO: choose for correct hitbox size
    sf::CircleShape shape(this->scale,3);
    shape.setOrigin(this->scale,this->scale);
    shape.move(this->pos);
    shape.scale(0.67,1.0);
    shape.rotate(180.0/M_PI*this->angle);
    shape.setFillColor(this->color);
    window->draw(shape);
}

// TODO: move to common
inline int get_type(uint16_t id) {
    if(id >= BLUE_TEAM_BEGIN && id < BLUE_BULLETS_BEGIN) return TYPE_SHIP;
    if(id >= BLUE_BULLETS_BEGIN && id < ASTEROIDS_BEGIN) return TYPE_BULLET;
    if(id >= ASTEROIDS_BEGIN && id <= TOTAL_ENTITIES) return TYPE_ASTEROID;
    return 0;
}

inline bool get_side(uint16_t id) {
    if(id >= BLUE_TEAM_BEGIN && id < RED_TEAM_BEGIN) return 0;
    if(id >= RED_TEAM_BEGIN && id < BLUE_BULLETS_BEGIN) return 1;
    if(id >= BLUE_BULLETS_BEGIN && id < RED_BULLETS_BEGIN) return 0;
    if(id >= RED_BULLETS_BEGIN && id < ASTEROIDS_BEGIN) return 1;
    return 0;
}

void Drawer::add(Movable* movable) {
    Shape* shape;
    uint8_t type = get_type(movable->id);
    bool side = get_side(movable->id);
    if(type==TYPE_SHIP) shape = new Triangle(
        sf::Vector2f(movable->position.x,movable->position.y),
        SHIP_SIZE,
        ((Ship*)movable)->direction,
        sf::Color(side?255:0,0,side?0:255,255)
    );
    if(type==TYPE_BULLET) shape = new Circle(
        sf::Vector2f(movable->position.x,movable->position.y),
        BULLET_SIZE,
        sf::Color(side?255:0,0,side?0:255,255)
    );
    if(type==TYPE_ASTEROID) shape = new Circle(
        sf::Vector2f(movable->position.x,movable->position.y),
        ASTEROID_SIZE,
        sf::Color(63,63,63,255)
    );
    this->shapes.push_back(shape);
}

void Drawer::addAll(DrawDataI* source) {
    std::vector<Movable*> data;
    source->getMovables(&data);
    for(Movable* movable: data) this->add(movable);
    for(Movable* movable: data) delete movable;
}

void Drawer::clear() {
    for(Shape* shape: this->shapes) delete shape;
    this->shapes.clear();
}

void Drawer::draw(sf::RenderWindow* window) {
    for(Shape* shape: this->shapes) shape->draw(window);
}

Drawer::~Drawer() { this->clear(); }



void GameState::getMovables(std::vector<Movable*>* movables) {
    std::lock_guard<std::mutex> lock(this->mtx);
    for(int i = 0; i < TOTAL_ENTITIES; ++i) {
        if(this->movables_present[i]) {
            // TODO: check if these are deep copies
            movables->push_back(new Movable(this->movables[i]));
        }
    }
}

void GameState::setMovables(uint32_t timestamp, std::vector<Movable*>* movables) {
    std::lock_guard<std::mutex> lock(this->mtx);
    if(this->timestamp > timestamp) return;
    this->timestamp = timestamp;
    memset(this->movables_present, false, TOTAL_ENTITIES*sizeof(bool));
    for(Movable* movable: *movables) {
        int i = movable->id-1;
        this->movables_present[i] = true;
        this->movables[i] = *movable;
        // delete movable;
    }
    // delete movables;
}

bool GameState::is_game_running() {
    // TODO
    return true;
}

UdpRecvData* UdpInputTranslator(uint8_t* data, int size) {
    if(size < 18) return NULL;
    uint32_t timestamp = *((uint32_t*)data);
    uint16_t blue_hp = *((uint16_t*)(data+4));
    uint16_t red_hp = *((uint16_t*)(data+6));
    uint16_t ammo = *((uint16_t*)(data+8));
    uint16_t reload = *((uint16_t*)(data+10));
    uint16_t rearm = *((uint16_t*)(data+12));
    uint16_t respawn = *((uint16_t*)(data+14));
    // uint16_t last_id = *((uint16_t*)(data+16));
    uint16_t movables_count = (size-18)/7;
    std::vector<Movable*>* movables = new std::vector<Movable*>;
    for(int i = 0; i < movables_count; ++i) {
        uint16_t id = *((uint16_t*)(data+18+7*i));
        uint16_t x0 = *((uint16_t*)(data+18+7*i+2));
        uint16_t y0 = *((uint16_t*)(data+18+7*i+4));
        uint8_t data_val = *((uint8_t*)(data+18+7*i+6));
        double x = 2.0 * (double) x0 / UINT16_MAX * TOTAL_RADIUS - TOTAL_RADIUS;
        double y = 2.0 * (double) y0 / UINT16_MAX * TOTAL_RADIUS - TOTAL_RADIUS;
        double direction = (double)(data_val & 127) / 128.0 * 2.0 * M_PI;
        // movables->push_back(new Asteroid(id, ))
    }
    return new UdpRecvData {
        // TODO
    };
}
