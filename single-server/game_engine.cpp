#include "game_engine.hpp"
#include <vector>
#include <cmath>
#include <ctime>


PlayerGE::PlayerGE(int player_id, Ship* ship):id(player_id),ship(ship),
ammo(MAX_AMMO),reload(0),rearm(0),respawn(RESPAWN_TIME),current_bullet(0),
last_input(GameIn{0,(float)(ship->id<RED_TEAM_BEGIN?M_PI_2:-M_PI_2),false,false}){
    ship->player = NULL;
}

PlayerGE::PlayerGE():id(0),ship(NULL),
ammo(MAX_AMMO),reload(0),rearm(0),respawn(RESPAWN_TIME),current_bullet(0),
last_input(GameIn{0,0.f,false,false}){}


inline double reduce_speed(double x) {
    double s = sgn(x);
    if(fabs(x) < 0.5) return x;
    if(fabs(x) > 1.5) return s;
    return 0.25*s + 0.5 * x;
    return s - 0.5 * s * (x - 1.5 * s) * (x - 1.5 * s);
}
void PlayerGE::update_ship() {
    if(this->respawn) {--this->respawn; return;}
    double angle = -this->last_input.angle;
    double da = SHIP_ROTATION * (sin(angle-this->ship->angle));
    double ship_acceleration = this->last_input.engine_on ? SHIP_ACCELERATION : 0.0;
    double vx = this->ship->speed.x + ship_acceleration * cos(this->ship->angle);
    double vy = this->ship->speed.y + ship_acceleration * sin(this->ship->angle);
    double v = sqrt(vx*vx+vy*vy);
    double v_coef = SHIP_SPEED * reduce_speed(v/SHIP_SPEED);
    double vxx = v == 0.0 ? 0.0 : vx/v;
    double vyy = v == 0.0 ? 0.0 : vy/v;
    this->ship->speed.x = vxx * v_coef;
    this->ship->speed.y = vyy * v_coef;
    this->ship->angle = fmod(this->ship->angle+da+M_PI,2*M_PI)-M_PI;
}

void PlayerGE::shoot(Movables* movables) {
    if(this->last_input.shoot && this->ammo && !this->reload) {
        movables->shoot(this);
        --this->ammo;
        this->current_bullet = (this->current_bullet + 1)%BULLETS_PER_PLAYER;
        this->reload = RELOAD_TIME;
    } if(this->reload) --this->reload;
}

PlayerData PlayerGE::generate_player_data() {
    return PlayerData {
        this->ammo,
        this->reload,
        this->rearm,
        this->respawn,
        this->ship->id
    };
}

void Collider::update_collisions(Movables* movables) {
    // ship - bullet
    for(int id1 = BLUE_TEAM_BEGIN; id1 < BLUE_BULLETS_BEGIN; ++id1) for(int id2 = BLUE_BULLETS_BEGIN; id2 < ASTEROIDS_BEGIN; ++id2) {
        Ship* ship = (Ship*)(movables->items[id1-1]);
        if(!ship->player) continue;
        Bullet* bullet = (Bullet*)(movables->items[id2-1]);
        vec2 p1 = ship->position; vec2 p2 = bullet->position;
        if((p1.x-p2.x)*(p1.x-p2.x)+(p1.y-p2.y)*(p1.y-p2.y)<(SHIP_SIZE+BULLET_SIZE)*(SHIP_SIZE+BULLET_SIZE)) {
            movables->respawn(ship); bullet->position = vec2{INFINITY, INFINITY}; bullet->lifetime = 0;
        }
    }
    // ship - asteroid
    for(int id1 = BLUE_TEAM_BEGIN; id1 < BLUE_BULLETS_BEGIN; ++id1) for(int id2 = ASTEROIDS_BEGIN; id2 <= TOTAL_ENTITIES; ++id2) {
        Ship* ship = (Ship*)(movables->items[id1-1]);
        if(!ship->player) continue;
        Asteroid* asteroid = (Asteroid*)(movables->items[id2-1]);
        vec2 p1 = ship->position; vec2 p2 = asteroid->position;
        if((p1.x-p2.x)*(p1.x-p2.x)+(p1.y-p2.y)*(p1.y-p2.y)<(SHIP_SIZE+ASTEROID_SIZE)*(SHIP_SIZE+ASTEROID_SIZE)) {
            movables->respawn(ship); movables->add_asteroid(id2);
        }
    }
    // bullet - asteroid
    for(int id1 = BLUE_BULLETS_BEGIN; id1 < ASTEROIDS_BEGIN; ++id1) for(int id2 = ASTEROIDS_BEGIN; id2 <= TOTAL_ENTITIES; ++id2) {
        Bullet* bullet = (Bullet*)(movables->items[id1-1]);
        if(!bullet->lifetime) continue;
        Asteroid* asteroid = (Asteroid*)(movables->items[id2-1]);
        vec2 p1 = bullet->position; vec2 p2 = asteroid->position;
        if((p1.x-p2.x)*(p1.x-p2.x)+(p1.y-p2.y)*(p1.y-p2.y)<(BULLET_SIZE+ASTEROID_SIZE)*(BULLET_SIZE+ASTEROID_SIZE)) {
            bullet->position = vec2{INFINITY, INFINITY}; bullet->lifetime = 0; movables->add_asteroid(id2);
        }
    }
}

void Collider::update_base(Movables* movables, Base* base, vec2 where, bool side) {
    // asteroids
    for(int id = ASTEROIDS_BEGIN; id < TOTAL_ENTITIES; ++id) {
        vec2 pos = movables->items[id-1]->position;
        if((pos.x-where.x)*(pos.x-where.x)+(pos.y-where.y)*(pos.y-where.y)<(BASE_ZONE_RADIUS+ASTEROID_SIZE)*(BASE_ZONE_RADIUS+ASTEROID_SIZE))
            movables->add_asteroid(id);
    }
    // bullets
    for(int id = BLUE_BULLETS_BEGIN; id < ASTEROIDS_BEGIN; ++id) {
        vec2 pos = movables->items[id-1]->position;
        if((pos.x-where.x)*(pos.x-where.x)+(pos.y-where.y)*(pos.y-where.y)<(BASE_RADIUS+BULLET_SIZE)*(BASE_RADIUS+BULLET_SIZE)) {
            movables->items[id-1]->position = vec2{INFINITY, INFINITY};
            if(base->hp) --base->hp;
        }
    }
    // reload
    for(int id = BLUE_TEAM_BEGIN; id < RED_TEAM_BEGIN; ++id) {
        vec2 pos = movables->items[id-1]->position;
        if((pos.x-where.x)*(pos.x-where.x)+(pos.y-where.y)*(pos.y-where.y)<BASE_RADIUS*BASE_RADIUS && get_side(id) == side) {
            PlayerGE* player = ((PlayerGE*)((Ship*)movables->items[id-1])->player);
            if(player == NULL) continue;
            if(player->rearm) --player->rearm;
            else if(player->ammo < MAX_AMMO) {
                ++player->ammo;
                player->rearm = RELOAD_TIME;
            }
        }
    }
}

Neighbours Collider::get_neighbours(Movables* movables, PlayerGE* player) {
    std::vector<SpaceObject*>* objects = new std::vector<SpaceObject*>;
    vec2 sp = player->ship->position;
    for(uint8_t id = BLUE_TEAM_BEGIN; id < TOTAL_ENTITIES; ++id) {
        Movable* movable = movables->items[id-1];
        vec2 pos = movable->position;
        if((pos.x-sp.x)*(pos.x-sp.x)+(pos.y-sp.y)*(pos.y-sp.y)<SIGHT_LIMIT*SIGHT_LIMIT) {
            int type = get_type(id);
            if(type == TYPE_SHIP && ((Ship*)movable)->player == NULL) continue;
            double angle = type == TYPE_SHIP ? ((Ship*)movable)->angle : get_type(id) == TYPE_BULLET ? ((Bullet*)movable)->angle : 0.0;
            objects->push_back(new SpaceObject{id, pos.x, pos.y, angle});
        }
    }
    uint8_t count = (uint8_t)objects->size();
    return Neighbours { count, objects };
}


Movables::Movables(unsigned int seed) {
    // team blue
    for(int i = BLUE_TEAM_BEGIN; i < RED_TEAM_BEGIN; ++i) {
        double angle = ((double) i / (double) PLAYERS_PER_TEAM) * 2.0 * M_PI;
        vec2 pos { SPAWN_RADIUS*cos(angle), BASE_DIST+SPAWN_RADIUS*sin(angle) };
        items[i-1] = new Ship(i, pos, vec2{0.0,0.0}, -M_PI_2, nullptr, false);
    }
    // team red
    for(int i = RED_TEAM_BEGIN; i < BLUE_BULLETS_BEGIN; ++i) {
        double angle = ((double) i / (double) PLAYERS_PER_TEAM) * 2.0 * M_PI;
        vec2 pos { -SPAWN_RADIUS*cos(angle), -BASE_DIST-SPAWN_RADIUS*sin(angle) };
        items[i-1] = new Ship(i, pos, vec2{0.0,0.0}, M_PI_2, nullptr, true);
    }
    // blue bullets
    for(int i = BLUE_BULLETS_BEGIN; i < RED_BULLETS_BEGIN; ++i) {
        items[i-1] = new Bullet(i, vec2{(double)BASE_DIST,0.0}, vec2{INFINITY, INFINITY}, -M_PI_2, 0, false);
    }
    // red bullets
    for(int i = RED_BULLETS_BEGIN; i < ASTEROIDS_BEGIN; ++i) {
        items[i-1] = new Bullet(i, vec2{-(double)BASE_DIST,0.0}, vec2{INFINITY, INFINITY}, M_PI_2, 0, true);
    }
    // asteroids
    for(int i = ASTEROIDS_BEGIN; i <= TOTAL_ENTITIES; ++i) {
        double radius = TOTAL_RADIUS * sqrt((double) rand_r(&seed) / (double) RAND_MAX);
        double angle = 2 * M_PI * ((double) rand_r(&seed) / (double) RAND_MAX);
        double pos_x = cos(angle) * radius;
        double pos_y = sin(angle) * radius;
        double speed_angle = 2 * M_PI * ((double) rand_r(&seed) / (double) RAND_MAX);
        double speed_x = ASTEROID_SPEED * cos(speed_angle);
        double speed_y = ASTEROID_SPEED * sin(speed_angle);
        items[i-1] = new Asteroid(i,  vec2{pos_x, pos_y}, vec2{speed_x, speed_y});
    }
    this->seed = seed;
}

void Movables::move(double dt) {
    for(Movable* movable: this->items) {
        movable->position.x += movable->speed.x * dt;
        movable->position.y += movable->speed.y * dt;
    }
}

#define RAD 5
void Movables::add_asteroid(uint16_t id) {
    double angle = 2 * M_PI * ((double) rand_r(&this->seed) / (double) RAND_MAX);
    double offset = 2 * BARRIER_RADIUS * ((double) rand_r(&this->seed) / (double) RAND_MAX) - BARRIER_RADIUS;
    double cooffset = sqrt(TOTAL_RADIUS*TOTAL_RADIUS - offset*offset);
    double position_x = offset * cos(angle) + cooffset * sin(angle);
    double position_y = cooffset * cos(angle) - offset * sin(angle);
    double speed_x = - ASTEROID_SPEED * sin(angle);
    double speed_y = - ASTEROID_SPEED * cos(angle);
    items[id-1]->position = vec2{position_x, position_y};
    items[id-1]->speed = vec2{speed_x, speed_y};
}

void Movables::shoot(PlayerGE* player) {
    uint16_t id = BLUE_BULLETS_BEGIN + (player->ship->id - 1) * BULLETS_PER_PLAYER + player->current_bullet;
    Bullet* bullet = (Bullet*) this->items[id-1];
    bullet->position = player->ship->position;
    bullet->position.x += (SHIP_SIZE+BULLET_SIZE+EPSILON)*cos(player->ship->angle);
    bullet->position.y += (SHIP_SIZE+BULLET_SIZE+EPSILON)*sin(player->ship->angle);
    bullet->speed = player->ship->speed;
    bullet->speed.x += BULLET_SPEED * cos(player->ship->angle);
    bullet->speed.y += BULLET_SPEED * sin(player->ship->angle);
    bullet->angle = player->ship->angle;    
    bullet->lifetime = BULLET_LIFETIME;
}

void Movables::update_asteroids() {
    for(uint16_t id = ASTEROIDS_BEGIN; id <= TOTAL_ENTITIES; ++id) {
        vec2 pos = this->items[id-1]->position;
        if(pos.x*pos.x+pos.y*pos.y>TOTAL_RADIUS*TOTAL_RADIUS+EPSILON) add_asteroid(id);
    }
}

void Movables::update_bullets() {
    for(uint16_t i = BLUE_BULLETS_BEGIN; i < ASTEROIDS_BEGIN; ++i) {
        if(((Bullet*)this->items[i-1])->lifetime) --((Bullet*)this->items[i-1])->lifetime;
        else this->items[i-1]->position = vec2{INFINITY, INFINITY};
    }
}

void Movables::respawn(Ship* ship) {
    ship->angle = ship->id < RED_TEAM_BEGIN ? -M_PI_2 : M_PI_2;
    double angle = ((double) ship->id / (double) PLAYERS_PER_TEAM) * 2.0 * M_PI;
    ship->position = ship->id < RED_TEAM_BEGIN ?
        vec2{ SPAWN_RADIUS*cos(angle), BASE_DIST+SPAWN_RADIUS*sin(angle) } :
        vec2{ -SPAWN_RADIUS*cos(angle), -BASE_DIST-SPAWN_RADIUS*sin(angle) };
    ship->speed = vec2{0.0,0.0};
    if(((PlayerGE*)ship->player) == NULL) return;
    ((PlayerGE*)ship->player)->ammo = MAX_AMMO;
    ((PlayerGE*)ship->player)->respawn = RESPAWN_TIME;
}

void Movables::pull_ships(double dt) {
    for(int id = BLUE_TEAM_BEGIN; id < BLUE_BULLETS_BEGIN; ++id) {
        Movable* ship = this->items[id-1];
        double x = ship->position.x;
        double y = ship->position.y;
        double r = sqrt(x*x+y*y);
        if(r>OUTER_RADIUS) {
            double dr = (r-OUTER_RADIUS)/(BARRIER_RADIUS-OUTER_RADIUS);
            double dx = SHIP_SPEED*x/r*dr*dt;
            double dy = SHIP_SPEED*y/r*dr*dt;
            ship->speed.x -= dx;
            ship->speed.y -= dy;
        }
    }
}

Movables::~Movables() {
    for(int i = 0; i < TOTAL_ENTITIES; ++i) delete this->items[i];
}

inline int rand_from_time() {
    timespec t;
    clock_gettime(CLOCK_MONOTONIC, &t);
    return t.tv_sec ^ t.tv_nsec;
}
GameEngine::GameEngine():timestamp(0),movables(rand_from_time()) {
    this->grid = Collider();
    this->blue.base.hp = BASE_HP;
    this->blue.size = 0;
    for(int id = BLUE_TEAM_BEGIN; id < RED_TEAM_BEGIN; ++id) {
        this->blue.players[id-BLUE_TEAM_BEGIN] = PlayerGE(-1, (Ship*) this->movables.items[id-1]);
    }
    this->red.base.hp = BASE_HP;
    this->red.size = 0;
    for(int id = RED_TEAM_BEGIN; id < BLUE_BULLETS_BEGIN; ++id) {
        this->red.players[id-RED_TEAM_BEGIN] = PlayerGE(-1, (Ship*) this->movables.items[id-1]);
    }
}

int GameEngine::update_physics(double dt) {
    ++this->timestamp;
    for(int i = 0; i < PLAYERS_PER_TEAM; ++i) {
        PlayerGE* player = this->blue.players + i;
        player->update_ship();
        player->shoot(&this->movables);
    }
    for(int i = 0; i < PLAYERS_PER_TEAM; ++i) {
        PlayerGE* player = this->red.players + i;
        player->update_ship();
        player->shoot(&this->movables);
    }
    this->movables.move(dt);
    this->movables.update_asteroids();
    this->movables.pull_ships(dt);
    this->movables.update_bullets();
    this->grid.update_base(&this->movables,&this->blue.base,vec2{0,BASE_DIST},false);
    this->grid.update_base(&this->movables,&this->red.base,vec2{0,-BASE_DIST},true);
    this->grid.update_collisions(&this->movables);
    return this->get_winner();
}

int GameEngine::get_winner() {
    return !this->red.base.hp ? BLUE_WIN :
        !this->blue.base.hp ? RED_WIN :
        !this->timestamp ? DRAW : NO_WIN;
}

void GameEngine::update_input(int ship_id, GameIn input) {
    PlayerGE* player = get_player(ship_id);
    if(player == NULL) return;
    if(input.timestamp > player->last_input.timestamp)
        player->last_input = input;
}

GameOut GameEngine::get_output(int ship_id) {
    PlayerGE* player = get_player(ship_id);
    uint32_t timestamp = this->timestamp;
    Base blue = this->blue.base;
    Base red = this->red.base;
    PlayerData player_data = player != NULL ? player->generate_player_data() : PlayerData {0,0,0,0,0};
    Neighbours neighbours = player != NULL ? this->grid.get_neighbours(&this->movables,player) : Neighbours {0, new std::vector<SpaceObject*>};
    return GameOut {
        timestamp,
        blue.hp,
        red.hp,
        player_data.ammo,
        player_data.reload,
        player_data.rearm,
        player_data.respawn,
        player_data.ship_id,
        neighbours.count,
        neighbours.movables
    };
}

void GameEngine::set_ship(int ship_id) {
    if(BLUE_TEAM_BEGIN <= ship_id && ship_id < RED_TEAM_BEGIN) {
        this->blue.players[ship_id-BLUE_TEAM_BEGIN].ship->player = this->blue.players+ship_id-BLUE_TEAM_BEGIN;
    } else if(RED_TEAM_BEGIN <= ship_id && ship_id < BLUE_BULLETS_BEGIN) {
        this->red.players[ship_id-RED_TEAM_BEGIN].ship->player = this->blue.players+ship_id-RED_TEAM_BEGIN;
    }
}

void GameEngine::unset_ship(int ship_id) {
    if(BLUE_TEAM_BEGIN <= ship_id && ship_id < RED_TEAM_BEGIN) {
        this->blue.players[ship_id-BLUE_TEAM_BEGIN].ship->player = NULL;
    } else if(RED_TEAM_BEGIN <= ship_id && ship_id < BLUE_BULLETS_BEGIN) {
        this->red.players[ship_id-RED_TEAM_BEGIN].ship->player = NULL;
    }
}

PlayerGE* GameEngine::get_player(int player_id) {
    // linear search - if teams get big this should be improved
    for(int i = 0; i < PLAYERS_PER_TEAM; ++i)
        if(this->blue.players[i].ship->id == player_id)
            return this->blue.players+i;
    for(int i = 0; i < PLAYERS_PER_TEAM; ++i)
        if(this->red.players[i].ship->id == player_id)
            return this->red.players+i;
    return NULL;
}
