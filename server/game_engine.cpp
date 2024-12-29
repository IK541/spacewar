#include "game_engine.hpp"
#include <vector>
#include <cmath>
#include <ctime>


Player::Player(int player_id, Ship* ship):id(player_id),ship(ship),
ammo(MAX_AMMO),reload(0),rearm(0),respawn(RESPAWN_TIME),current_bullet(0),
last_input(GameIn{0,(float)(ship->id<RED_TEAM_BEGIN?M_PI_2:-M_PI_2),false,false}){
    ship->player = this;
}

Player::Player():id(0),ship(NULL),
ammo(MAX_AMMO),reload(0),rearm(0),respawn(RESPAWN_TIME),current_bullet(0),
last_input(GameIn{0,0.f,false,false}){}


inline double reduce_speed(double x) {
    double s = sgn(x);
    if(fabs(x) < 0.5) return x;
    if(fabs(x) > 1.5) return s;
    return 0.25*s + 0.5 * x;
    return s - 0.5 * s * (x - 1.5 * s) * (x - 1.5 * s);
}
void Player::update_ship() {
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

void Player::shoot(Movables* movables) {
    if(this->last_input.shoot && this->ammo && !this->reload) {
        movables->shoot(this);
        --this->ammo;
        this->current_bullet = (this->current_bullet + 1)%BULLETS_PER_PLAYER;
        this->reload = RELOAD_TIME;
    } if(this->reload) --this->reload;
}

PlayerData Player::generate_player_data() {
    return PlayerData {
        .ammo = this->ammo,
        .reload = this->reload,
        .rearm = this->rearm,
        .respawn = this->respawn,
        .ship_id = this->ship->id
    };
}

#define CHECK_XY(x,y) if((x) >= 0 && (y) >= 0 && (x) < GRID_SIZE && (y) < GRID_SIZE)
void Grid::update_state(Movables* movables) {
    for(int y = 0; y < GRID_SIZE; ++y) {
        for(int x = 0; x < GRID_SIZE; ++x) {
            this->fields[GRID_SIZE*y+x].clear();
        }
    }
    for(int id = BLUE_TEAM_BEGIN; id <= TOTAL_ENTITIES; ++id) {
        vec2 pos = movables->items[id-1]->position;
        int x = (int) (pos.x + TOTAL_RADIUS);
        int y = (int) (pos.y + TOTAL_RADIUS);
        CHECK_XY(x,y) this->fields[GRID_SIZE*y+x].insert(movables->items[id-1]);
    }
}

inline void update_collisions_square(Movable* movable, std::set<Movable*>* movset, Movables* movables) {
    int id = movable->id;
    float s = get_size(id);
    vec2 pos = movable->position;
    double x = pos.x + TOTAL_RADIUS;
    double y = pos.y + TOTAL_RADIUS;
    int type = get_type(id);

    for(Movable* movable2: *movset) {
        int id2 = movable2->id;
        float s2 = get_size(id2);
        vec2 pos2 = movable2->position;
        double x2 = pos2.x + TOTAL_RADIUS;
        double y2 = pos2.y + TOTAL_RADIUS;
        int type2 = get_type(id2);

        if((x-x2)*(x-x2)+(y-y2)*(y-y2) < (s+s2)*(s+s2) && type != type2) {
            if(type == TYPE_SHIP) movables->respawn((Ship*)movable);
            if(type == TYPE_BULLET) movable->position = vec2{INFINITY, INFINITY};
            if(type == TYPE_ASTEROID) movables->add_asteroid(id);
            if(type2 == TYPE_SHIP) movables->respawn((Ship*)movable2);
            if(type2 == TYPE_BULLET) movable2->position = vec2{INFINITY, INFINITY};
            if(type2 == TYPE_ASTEROID) movables->add_asteroid(id2);
        }
    }
}
void Grid::update_collisions(Movables* movables) {
    for(int id = BLUE_TEAM_BEGIN; id < ASTEROIDS_BEGIN; ++id) {
        Movable* movable = movables->items[id-1];
        vec2 pos = movable->position;
        int x = (int) (pos.x + TOTAL_RADIUS);
        int y = (int) (pos.y + TOTAL_RADIUS);
        double shift_x = fmod(x, 1.0);
        double shift_y = fmod(y, 1.0);
        double min_x = shift_x - get_size(id);
        double min_y = shift_y - get_size(id);
        double max_x = shift_x + get_size(id);
        double max_y = shift_y + get_size(id);
        CHECK_XY(x,y) update_collisions_square(movable, &(this->fields[GRID_SIZE*y+x]), movables);
        if(min_y < 0.0) CHECK_XY(x, y-1) update_collisions_square(movable, &(this->fields[GRID_SIZE*(y-1)+x]), movables);
        if(min_y < 0.0 && max_x > 1.0) CHECK_XY(x+1, y-1) update_collisions_square(movable, &(this->fields[GRID_SIZE*(y-1)+x+1]), movables);
        if(max_x > 1.0) CHECK_XY(x+1, y) update_collisions_square(movable, &(this->fields[GRID_SIZE*y+x+1]), movables);
        if(max_y > 1.0 && max_x > 1.0) CHECK_XY(x+1, y+1) update_collisions_square(movable, &(this->fields[GRID_SIZE*(y+1)+x+1]), movables);
        if(max_y > 1.0) CHECK_XY(x, y+1) update_collisions_square(movable, &(this->fields[GRID_SIZE*(y+1)+x]), movables);
        if(max_y > 1.0 && min_x < 0.0) CHECK_XY(x-1, y+1) update_collisions_square(movable, &(this->fields[GRID_SIZE*(y+1)+x-1]), movables);
        if(min_x < 0.0) CHECK_XY(x-1, y) update_collisions_square(movable, &(this->fields[GRID_SIZE*y+x-1]), movables);
        if(min_y < 0.0 && min_x < 0.0) CHECK_XY(x-1, y-1) update_collisions_square(movable, &(this->fields[GRID_SIZE*(y-1)+x-1]), movables);
    }
}

void Grid::update_zone(Movables* movables, vec2 where) {
    for(int y = TOTAL_RADIUS+where.y-BASE_ZONE_RADIUS-1; y <= TOTAL_RADIUS+where.y+BASE_ZONE_RADIUS; ++y) {
        for(int x = TOTAL_RADIUS+where.x-BASE_ZONE_RADIUS-1; x <= TOTAL_RADIUS+where.x+BASE_ZONE_RADIUS; ++x) {
            CHECK_XY(x,y) for (Movable* movable: this->fields[y*GRID_SIZE+x]) {
                int id = movable->id;
                vec2 pos = movable->position;
                if(get_type(id) == TYPE_ASTEROID)
                if((pos.x-where.x)*(pos.x-where.x)+(pos.y-where.y)*(pos.y-where.y)<(BASE_ZONE_RADIUS+ASTEROID_SIZE)*(BASE_ZONE_RADIUS+ASTEROID_SIZE)) {
                    movables->add_asteroid(id);
                }
            }
        }
    }
}

void Grid::update_base(Base* base, vec2 where, bool side) {
    for(int y = TOTAL_RADIUS+where.y-BASE_RADIUS-1; y <= TOTAL_RADIUS+where.y+BASE_RADIUS; ++y) {
        for(int x = TOTAL_RADIUS+where.x-BASE_RADIUS-1; x <= TOTAL_RADIUS+where.x+BASE_RADIUS; ++x) {
            CHECK_XY(x,y) for(Movable* movable: this->fields[y*GRID_SIZE+x]) {
                int id = movable->id;
                vec2 pos = movable->position;
                if(get_type(id) == TYPE_BULLET)
                if((pos.x-where.x)*(pos.x-where.x)+(pos.y-where.y)*(pos.y-where.y)<(BASE_RADIUS+BULLET_SIZE)*(BASE_RADIUS+BULLET_SIZE)) {
                    movable->position = vec2{INFINITY, INFINITY};
                    if(base->hp) --base->hp;
                }
                if(get_type(id) == TYPE_SHIP && get_side(id) == side)
                if((pos.x-where.x)*(pos.x-where.x)+(pos.y-where.y)*(pos.y-where.y)<BASE_RADIUS*BASE_RADIUS) {
                    Player* player = ((Player*)((Ship*)movable)->player);
                    if(player->rearm) --player->rearm;
                    else if(player->ammo < MAX_AMMO) {
                        ++player->ammo;
                        player->rearm = RELOAD_TIME;
                    }
                }
            }
        }
    }
}

Neighbours Grid::getNeighbours(Player* player) {
    std::vector<SpaceObject*>* movables = new std::vector<SpaceObject*>;
    int sy = (int)player->ship->position.y;
    int sx = (int)player->ship->position.x;
    for(int y = TOTAL_RADIUS+sy-SIGHT_LIMIT-1; y <= TOTAL_RADIUS+sy+SIGHT_LIMIT; ++y)
        for(int x = TOTAL_RADIUS+sx-SIGHT_LIMIT-1; x <= TOTAL_RADIUS+sx+SIGHT_LIMIT; ++x)
            CHECK_XY(x,y) for(Movable* movable: this->fields[y*GRID_SIZE+x]) {
                double angle = get_type(movable->id) == TYPE_SHIP ? ((Ship*)movable)->angle :
                    get_type(movable->id) == TYPE_BULLET ? ((Bullet*)movable)->angle : 0.0;
                movables->push_back(new SpaceObject{movable->id, movable->position.x, movable->position.y, angle});
            }
    uint8_t count = (uint8_t)movables->size();
    return Neighbours {
        .count = count,
        .movables = movables
    };
}
#undef CHECK_XY


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

void Movables::shoot(Player* player) {
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
    ((Player*)ship->player)->ammo = MAX_AMMO;
    ((Player*)ship->player)->respawn = RESPAWN_TIME;
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
            ship->position.x -= dx;
            ship->position.y -= dy;
        }
    }
}

GameEngine::GameEngine():timestamp(0),movables(time(NULL)) {
    this->grid = Grid();
    this->blue.base.hp = BASE_HP;
    this->blue.size = 0;
    for(int id = BLUE_TEAM_BEGIN; id < RED_TEAM_BEGIN; ++id) {
        this->blue.players[id-BLUE_TEAM_BEGIN] = Player(-1, (Ship*) this->movables.items[id-1]);
        this->blue.players[id-BLUE_TEAM_BEGIN].ship->player = this->blue.players+id-BLUE_TEAM_BEGIN;
    }
    this->red.base.hp = BASE_HP;
    this->red.size = 0;
    for(int id = RED_TEAM_BEGIN; id < BLUE_BULLETS_BEGIN; ++id) {
        this->red.players[id-RED_TEAM_BEGIN] = Player(-1, (Ship*) this->movables.items[id-1]);
        this->red.players[id-RED_TEAM_BEGIN].ship->player = this->red.players+id-RED_TEAM_BEGIN;
    }
}

void GameEngine::update_physics(double dt) {
    ++this->timestamp;
    for(int i = 0; i < PLAYERS_PER_TEAM; ++i) {
        Player* player = this->blue.players + i;
        player->update_ship();
        player->shoot(&this->movables);
    }
    for(int i = 0; i < PLAYERS_PER_TEAM; ++i) {
        Player* player = this->red.players + i;
        player->update_ship();
        player->shoot(&this->movables);
    }
    this->movables.move(dt);
    this->movables.update_asteroids();
    this->movables.pull_ships(dt);
    this->movables.update_bullets();
    this->grid.update_state(&this->movables);
    this->grid.update_collisions(&this->movables);
    this->grid.update_zone(&this->movables,vec2{0,BASE_DIST});
    this->grid.update_zone(&this->movables,vec2{0,-BASE_DIST});
    this->grid.update_base(&this->blue.base,vec2{0,BASE_DIST},false);
    this->grid.update_base(&this->red.base,vec2{0,-BASE_DIST},true);
}

void GameEngine::update_input(int ship_id, GameIn input) {
    Player* player = get_player(ship_id);
    if(input.timestamp > player->last_input.timestamp)
        player->last_input = input;
}

GameOut GameEngine::get_output(int ship_id) {
    Player* player = get_player(ship_id);
    uint32_t timestamp = this->timestamp;
    Base blue = this->blue.base;
    Base red = this->red.base;
    PlayerData player_data = player->generate_player_data();
    Neighbours neighbours = this->grid.getNeighbours(player);
    return GameOut {
        .timestamp = timestamp,
        .blue_hp = blue.hp,
        .red_hp = red.hp,
        .ammo = player_data.ammo,
        .reload = player_data.reload,
        .rearm = player_data.rearm,
        .respawn = player_data.respawn,
        .ship_id = player_data.ship_id,
        .movables_count = neighbours.count,
        .objects = neighbours.movables
    };
}

Player* GameEngine::get_player(int player_id) {
    // linear search - if teams get big this should be improved
    for(int i = 0; i < PLAYERS_PER_TEAM; ++i)
        if(this->blue.players[i].ship->id == player_id)
            return this->blue.players+i;
    for(int i = 0; i < PLAYERS_PER_TEAM; ++i)
        if(this->red.players[i].ship->id == player_id)
            return this->red.players+i;
    return NULL;
}
