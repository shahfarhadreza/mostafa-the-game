#ifndef _GAME_H_
#define _GAME_H_

#define CHARACTER_STATE_IDLE 1
#define CHARACTER_STATE_WALKING 2

#define GAME_STATE_MENU 0
#define GAME_STATE_PLAYING 1
#define GAME_STATE_PAUSED 2
#define GAME_STATE_OVER 3
#define GAME_STATE_WIN 4

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
    HWND hwnd;
    wchar_t title[256];
    int height;
    int width;
    int top_margin;
    HBITMAP dbl_buffer;
    HFONT font;
    HFONT font_big;
    double time_elapsed;

    int state;
    char keyboard_state[256];

    struct player_t player;
};

#endif

