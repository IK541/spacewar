#include "draw.hpp"
#include <cmath>

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

void Drawer::clear() {
    for(Shape* shape: this->shapes) delete shape;
    this->shapes.clear();
}

void Drawer::draw(sf::RenderWindow* window) {
    for(Shape* shape: this->shapes) shape->draw(window);
}

Drawer::~Drawer() { this->clear(); }
