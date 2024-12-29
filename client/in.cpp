#include "in.hpp"

#include <cstring>

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

void Drawer::add(SpaceObject* object) {
    Shape* shape;
    uint8_t type = get_type(object->id);
    bool side = get_side(object->id);
    if(type==TYPE_SHIP) shape = new Triangle(
        sf::Vector2f(object->x,object->y),
        SHIP_SIZE,
        sf::Color(side?255:0,0,side?0:255,255),
        object->angle
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

void Drawer::add_all(DrawDataI* source) {
    DrawData data = source->get_game_state();
    for(SpaceObject* object: *data.objects) this->add(object);
    this->blue = data.blue; this->red = data.red;
    this->ammo = data.ammo; this->respawn = data.respawn;
    for(SpaceObject* obj: *data.objects) delete obj;
    delete data.objects;
}

void Drawer::clear() {
    for(Shape* shape: this->shapes) delete shape;
    this->shapes.clear();
}

void Drawer::draw(WindowData window) {
    this->draw_bases(window);
    this->draw_objects(window);
    this->draw_zones(window);
    this->draw_hp(window);
    this->draw_ammo(window);
    this->draw_respawn(window);
}

Drawer::~Drawer() { this->clear(); }

void Drawer::draw_objects(WindowData window) {
    for(Shape* shape: this->shapes) shape->draw(window.window);
}

void Drawer::draw_zones(WindowData window) {
    Circle blue_zone(sf::Vector2f(0,BASE_DIST),BASE_ZONE_RADIUS,sf::Color(0,0,127,63)); blue_zone.draw(window.window);
    Circle red_zone(sf::Vector2f(0,-BASE_DIST),BASE_ZONE_RADIUS,sf::Color(127,0,0,63)); red_zone.draw(window.window);
}

void Drawer::draw_bases(WindowData window) {
    Circle blue_base(sf::Vector2f(0,BASE_DIST),BASE_RADIUS,sf::Color(0,0,127)); blue_base.draw(window.window);
    Circle red_base(sf::Vector2f(0,-BASE_DIST),BASE_RADIUS,sf::Color(127,0,0)); red_base.draw(window.window);
}

void Drawer::draw_hp(WindowData window) {
    sf::Vector2f v = get_view_size(window.size);

    float blue_len = 2.f * this->blue / BASE_HP * v.x;
    sf::RectangleShape blue_bar = sf::RectangleShape(sf::Vector2f(blue_len, HP_BAR_WIDTH));
    blue_bar.move(window.center+v-sf::Vector2f(blue_len,HP_BAR_WIDTH));
    blue_bar.setFillColor(sf::Color(0,0,255));
    window.window->draw(blue_bar);

    float red_len = 2.f * this->red / BASE_HP * v.x;
    sf::RectangleShape red_bar = sf::RectangleShape(sf::Vector2f(red_len, HP_BAR_WIDTH));
    red_bar.move(window.center-v);
    red_bar.setFillColor(sf::Color(255,0,0));
    window.window->draw(red_bar);
}

void Drawer::draw_ammo(WindowData window) {
    sf::Vector2f v = get_view_size(window.size);
    for(int i = 0; i < ammo; ++i) {
        sf::CircleShape ammo = sf::CircleShape(AMMO_RADIUS);
        ammo.move(window.center-v+sf::Vector2f(0,HP_BAR_WIDTH+AMMO_RADIUS+i*v.y/(MAX_AMMO+1)));
        ammo.setFillColor(sf::Color(127,127,0));
        window.window->draw(ammo);
    }
}

void Drawer::draw_respawn(WindowData window) {
    for(int i = 0; i < respawn; ++i) {
        sf::CircleShape respawn = sf::CircleShape(0.5f*RESPAWN_WIDTH);
        double angle = 2 * M_PI * ((double) i) / RESPAWN_TIME;
        respawn.move(window.center+sf::Vector2f(RESPAWN_RADIUS*sin(angle),RESPAWN_RADIUS*cos(angle)));
        respawn.setFillColor(sf::Color(127,127,127));
        window.window->draw(respawn);
    }
}



GameState::GameState():timestamp(0),ammo(0),respawn(0),center(sf::Vector2f(0.f,0.f)) {
    memset(this->objects_present,0,TOTAL_ENTITIES*sizeof(bool));
    memset(this->objects,0,TOTAL_ENTITIES*sizeof(SpaceObject));
}

void GameState::get_space_objects(std::vector<SpaceObject*>* objects) {
    for(int i = 0; i < TOTAL_ENTITIES; ++i) {
        if(this->objects_present[i]) {
            SpaceObject it = this->objects[i];
            objects->push_back(new SpaceObject{it.id,it.x,it.y,it.angle});
        }
    }
}

DrawData GameState::get_game_state() {
    std::lock_guard<std::mutex> lock(this->mtx);
    std::vector<SpaceObject*>* objects = new std::vector<SpaceObject*>;
    this->get_space_objects(objects);
    return DrawData {
        .objects = objects,
        .blue = this->blue_hp,
        .red = this->red_hp,
        .ammo = this->ammo,
        .respawn = this->respawn
    };
}

sf::Vector2f GameState::get_center() {
    std::lock_guard<std::mutex> lock(this->mtx);
    return this->center;
}

void GameState::set_game_state(GameOut out) {
    std::lock_guard<std::mutex> lock(this->mtx);
    if(this->timestamp > out.timestamp) return;
    this->timestamp = out.timestamp;
    this->blue_hp = out.blue_hp;
    this->red_hp = out.red_hp;
    this->ammo = out.ammo;
    this->respawn = out.respawn;
    uint8_t ship_id = out.ship_id;
    memset(this->objects_present, false, TOTAL_ENTITIES*sizeof(bool));
    for(SpaceObject* object: *out.objects) {
        int i = object->id-1;
        this->objects_present[i] = true;
        this->objects[i] = *object;
        if(object->id == ship_id) {
            this->center.x = object->x;
            this->center.y = object->y;
        }
    }
}

bool GameState::is_game_running() {
    // TODO - lobby & tcp
    return true;
}

GameOut UdpInputTranslator(uint8_t* data, int size) {
    uint32_t timestamp = *((uint32_t*)data);
    uint16_t blue_hp = *((uint16_t*)(data+4));
    uint16_t red_hp = *((uint16_t*)(data+6));
    uint16_t ammo = *((uint16_t*)(data+8));
    uint16_t reload = *((uint16_t*)(data+10));
    uint16_t rearm = *((uint16_t*)(data+12));
    uint16_t respawn = *((uint16_t*)(data+14));
    uint8_t ship_id = *((uint8_t*)(data+16));
    uint8_t movables_count = (size-18)/6;
    std::vector<SpaceObject*>* objects = new std::vector<SpaceObject*>;
    for(int i = 0; i < movables_count; ++i) {
        uint8_t id = *((uint8_t*)(data+18+6*i));
        uint16_t x0 = *((uint16_t*)(data+18+6*i+1));
        uint16_t y0 = *((uint16_t*)(data+18+6*i+3));
        uint8_t angle0 = *((uint8_t*)(data+18+6*i+5));
        double x = 2.0 * (double) x0 / UINT16_MAX * TOTAL_RADIUS - TOTAL_RADIUS;
        double y = 2.0 * (double) y0 / UINT16_MAX * TOTAL_RADIUS - TOTAL_RADIUS;
        double angle = (double)angle0 / 256.0 * 2.0 * M_PI;
        objects->push_back(new SpaceObject{id,x,y,angle});
    }
    return GameOut {
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
