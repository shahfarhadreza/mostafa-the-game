#ifndef _GAME_H_
#define _GAME_H_

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>
#include <windows.h>

#define CHARACTER_STATE_IDLE 1
#define CHARACTER_STATE_WALKING 2
#define CHARACTER_STATE_RUNNING 3
#define CHARACTER_STATE_FIGHT 4

#define CHARACTER_DIRECTION_RIGHT 1
#define CHARACTER_DIRECTION_LEFT -1

#define GAME_STATE_MENU 0
#define GAME_STATE_PLAYING 1
#define GAME_STATE_PAUSED 2
#define GAME_STATE_OVER 3
#define GAME_STATE_WIN 4

struct vector3d_t {
    float x, y, z;
};

struct animation_t {
    wchar_t name[256];
    int frame_start;
    int frame_end;
    float speed;
    int play_once;
    int reverse_loop;
    int flag;
};

struct sprite_t {
    wchar_t file_name[256];
    HBITMAP hbitmap;
    HBITMAP hbitmap_flip;
    int width;
    int height;
    float scale;
    int frames_per_row;
    int frame;
    int frame_x;
    int frame_y;
    int flag;
    struct animation_t* animations;
    int animation_max;
    int animation_count;
    int animation_current;
    double animation_time;
};

struct game_object_t {
    struct vector3d_t position;
};

struct enemy_t {
    struct vector3d_t position;
    struct vector3d_t velocity;
    float movement_speed;
    int direction;
    RECT collision_rect;
    float health;
    int state;
    struct sprite_t* sprite;
    double animation_time;
};

struct player_t {
    struct vector3d_t position;
    struct vector3d_t velocity;
    float movement_speed;
    int direction;
    RECT* hit_boxes;
    int hit_box_count;
    float health;
    int state;
    struct sprite_t* sprite;
    double animation_time;

    int score;
};

struct draw_t {
    HBITMAP hbitmap;
    struct vector3d_t position;
    int width;
    int height;
    int src_x;
    int src_y;
    int src_width;
    int src_height;
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

    struct player_t* player;

    struct enemy_t* enemies;
    int enemy_max;
    int enemy_count;

    HGDIOBJ bmp_bg;

    // things to draw on screen
    struct draw_t* draw_list;
    int draw_max;
    int draw_count;
};

extern struct game_t* game;

void vector3d_zero(struct vector3d_t* vec);

void graphics_clear();
void graphics_draw(struct draw_t* draw);

struct sprite_t* sprite_new(const wchar_t* filename, const wchar_t* filename_flip, int width, int height, float scale);
void sprite_delete(struct sprite_t* sprite);
void sprite_update(struct sprite_t* sprite, float dt);
void sprite_draw(struct sprite_t* sprite, struct vector3d_t* pos, int direction, HDC hdc);
struct animation_t* sprite_add_animation(struct sprite_t* sprite, const wchar_t* name, int start, int end, int reverse_loop);
void sprite_set_animation(struct sprite_t* sprite, const wchar_t* name);

struct player_t* player_new();
void player_delete(struct player_t* player);
void player_draw(struct player_t* player, HDC hdc);
void player_update(struct player_t* player, float dt);
void player_set_state(struct player_t* player, int state);

void enemy_spwan(int x, int y, int type);
void enemy_draw(struct enemy_t* enemy, HDC hdc);
void enemy_update(struct enemy_t* enemy, int index, float dt);
void enemy_cleanup(struct enemy_t* enemy);

static float get_randf(float a, float b) {
    float random = ((float) rand()) / (float) RAND_MAX;
    float diff = b - a;
    float r = random * diff;
    return a + r;
}

static int get_rand(int min, int max) {
    return rand()%(max-min + 1) + min;
}

static HFONT create_font(const wchar_t* name, int size) {
    HDC hDC = GetDC(HWND_DESKTOP);
    int hight = -MulDiv(size, GetDeviceCaps(hDC, LOGPIXELSY), 72);
    ReleaseDC(HWND_DESKTOP, hDC);
    HFONT font = CreateFontW(hight, 0, 0, 0, FW_BOLD, 0, 0, 0,
	   DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH, name);
    return font;
}

#endif

