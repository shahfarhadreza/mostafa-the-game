#include <windows.h>

#define CHARACTER_STATE_IDLE 1
#define CHARACTER_STATE_WALKING 2

struct vector3d_t {
    float x, y, z;
};

struct sprite_sheet_t {
    char* file_name;
};

struct player_t {
    struct vector3d_t position;
    float health;
    int score;
    int state;
};

struct game_t {
    struct player_t player;
};

void sprite_sheet_draw(struct sprite_sheet_t* sheet,  int index) {

}

void game_init() {

}

int main() {
    return 0;
}

