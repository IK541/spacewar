#include "common.hpp"
#include <cmath>

Shape::Shape(sf::Vector2f pos, float scale, sf::Color color):
pos(pos),scale(scale),color(color){}

Triangle::Triangle(sf::Vector2f pos, float scale, sf::Color color, float angle):
Shape(pos,scale,color),angle(angle){}

Circle::Circle(sf::Vector2f pos, float scale, sf::Color color):
Shape(pos,scale,color){}

void Circle::draw(sf::RenderWindow* window) {
    sf::CircleShape shape(this->scale);
    shape.setOrigin(this->scale, this->scale);
    shape.move(this->pos);
    shape.setFillColor(this->color);
    window->draw(shape);
}

void Triangle::draw(sf::RenderWindow* window) {
    sf::CircleShape shape(this->scale,3);
    shape.setOrigin(this->scale,this->scale);
    shape.move(this->pos);
    shape.scale(1.6,2.4);
    shape.move(0.f,-0.25f*this->scale);
    shape.rotate(180.0/M_PI*this->angle+90.0);
    shape.setFillColor(this->color);
    window->draw(shape);
}
