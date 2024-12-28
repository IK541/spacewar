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

void Drawer::add(SpaceObject* object) {
    Shape* shape;
    uint8_t type = get_type(object->id);
    bool side = get_side(object->id);
    if(type==TYPE_SHIP) shape = new Triangle(
        sf::Vector2f(object->x,object->y),
        SHIP_SIZE,
        object->angle,
        sf::Color(side?255:0,0,side?0:255,255)
    );
    if(type==TYPE_BULLET) shape = new Circle(
        sf::Vector2f(object->x,object->y),
        BULLET_SIZE,
        sf::Color(side?255:0,0,side?0:255,255)
    );
    if(type==TYPE_ASTEROID) shape = new Circle(
        sf::Vector2f(object->x,object->y),
        ASTEROID_SIZE,
        sf::Color(63,63,63,255)
    );
    this->shapes.push_back(shape);
}

void Drawer::addAll(DrawDataI* source) {
    std::vector<SpaceObject*> data;
    source->get_space_objects(&data);
    for(SpaceObject* object: data) this->add(object);
    for(SpaceObject* object: data) delete object;
}

void Drawer::clear() {
    for(Shape* shape: this->shapes) delete shape;
    this->shapes.clear();
}

void Drawer::draw(sf::RenderWindow* window) {
    for(Shape* shape: this->shapes) shape->draw(window);
}

Drawer::~Drawer() { this->clear(); }



void GameState::get_space_objects(std::vector<SpaceObject*>* objects) {
    std::lock_guard<std::mutex> lock(this->mtx);
    for(int i = 0; i < TOTAL_ENTITIES; ++i) {
        if(this->objects_present[i]) {
            SpaceObject it = this->objects[i];
            objects->push_back(new SpaceObject{it.id,it.x,it.y,it.angle});
        }
    }
}

void GameState::set_space_objects(uint32_t timestamp, std::vector<SpaceObject*>* objects) {
    std::lock_guard<std::mutex> lock(this->mtx);
    if(this->timestamp > timestamp) return;
    this->timestamp = timestamp;
    memset(this->objects_present, false, TOTAL_ENTITIES*sizeof(bool));
    for(SpaceObject* object: *objects) {
        int i = object->id-1;
        this->objects_present[i] = true;
        this->objects[i] = *object;
        // delete movable;
    }
    // delete movables;
}

bool GameState::is_game_running() {
    // TODO - lobby & tcp
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
    uint8_t ship_id = *((uint8_t*)(data+16));
    uint16_t movables_count = (size-18)/6;
    std::vector<SpaceObject*>* objects = new std::vector<SpaceObject*>;
    for(int i = 0; i < movables_count; ++i) {
        uint8_t id = *((uint8_t*)(data+18+6*i));
        uint16_t x0 = *((uint16_t*)(data+18+6*i+1));
        uint16_t y0 = *((uint16_t*)(data+18+6*i+3));
        uint8_t data_val = *((uint8_t*)(data+18+6*i+5));
        double x = 2.0 * (double) x0 / UINT16_MAX * TOTAL_RADIUS - TOTAL_RADIUS;
        double y = 2.0 * (double) y0 / UINT16_MAX * TOTAL_RADIUS - TOTAL_RADIUS;
        double angle = (double)data_val / 256.0 * 2.0 * M_PI;
        objects->push_back(new SpaceObject{id,x,y,angle});
    }
    return new UdpRecvData {
        .timestamp = timestamp,
        .blue_hp = blue_hp,
        .red_hp = red_hp,
        .ammo = ammo,
        .reload = reload,
        .rearm = rearm,
        .respawn = respawn,
        .ship_id = ship_id,
        .movables_count = movables_count,
        .objects = objects
    };
}
