#pragma once
#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>


class Shape {
    public:
    sf::Vector2f pos;
    float scale;
    sf::Color color;
    Shape(sf::Vector2f pos, float scale, sf::Color color);
    virtual ~Shape(){}
    virtual void draw(sf::RenderWindow* window) = 0;
};

class Circle : public Shape {
    public:
    Circle(sf::Vector2f pos, float scale, sf::Color color);
    void draw(sf::RenderWindow* window);
};

class Triangle : public Shape {
    public:
    float angle;
    Triangle(sf::Vector2f pos, float scale, sf::Color color, float angle);
    void draw(sf::RenderWindow* window);
};
