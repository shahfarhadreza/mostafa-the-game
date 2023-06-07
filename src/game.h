#ifndef _GAME_H_
#define _GAME_H_

#define CHARACTER_STATE_IDLE 1
#define CHARACTER_STATE_WALKING 2

struct vector3d_t {
    float x, y, z;
};

struct sprite_sheet_t {
    char* file_name;
};

struct game_object_t {
    struct vector3d_t position;
};

struct enemy_t {
    struct vector3d_t position;
    float health;
    int state;
};

struct player_t {
    struct vector3d_t position;
    float health;
    int state;
    int score;
};

struct game_t {
    struct player_t player;
};

#endif

