#pragma once
#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>
#include <vector>

#include "../common.hpp"

class Shape {
    public:
    sf::Vector2f pos;
    float scale;
    float angle;
    sf::Color color;
    Shape(sf::Vector2f pos, float scale, float angle, sf::Color color);
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
    Triangle(sf::Vector2f pos, float scale, float angle, sf::Color color);
    void draw(sf::RenderWindow* window);
};

class Drawer {
    public:
    std::vector<Shape*> shapes;
    // NEW
    void add(Movable* movable);
    void clear();
    void draw(sf::RenderWindow* window);
    ~Drawer();
};
