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
#define CHARACTER_STATE_PUNCH_PRE 4
#define CHARACTER_STATE_FIGHT 5
#define CHARACTER_STATE_HIT 6
#define CHARACTER_STATE_HIT_FALL 7
#define CHARACTER_STATE_MOVE 8
#define CHARACTER_STATE_JUMP 9
#define CHARACTER_STATE_RANGE_ATTACK 10
#define CHARACTER_STATE_WIN 11

#define CHARACTER_DIRECTION_RIGHT 1
#define CHARACTER_DIRECTION_LEFT -1

#define GAME_STATE_MENU 0
#define GAME_STATE_INTRO 1
#define GAME_STATE_CUTSCENE 2
#define GAME_STATE_PLAYING 3
#define GAME_STATE_PAUSED 4
#define GAME_STATE_OVER 5
#define GAME_STATE_WIN 6

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
    int animation_ended;
};

struct sprite_t {
    wchar_t file_name[256];
    HBITMAP hbitmap;
    COLORREF color_mask;
    int width;
    int height;
    float scale;
    int draw_top;
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

#define BBOX_MAIN 0
#define BBOX_HEAD 1
#define BBOX_CHEST 2
#define BBOX_FIST 3
#define BBOX_LEGS 4

struct character_t {
    wchar_t name[256];
    struct vector3d_t position;
    struct vector3d_t velocity;
    int on_ground;
    int direction;
    int prev_state;
    int state;
    int state_on_enter;
    struct sprite_t* sprite;
    float health;
    RECT* hit_boxes;
    int hit_box_count;
    int visible;
};

#define ENEMY_TYPE_FERRIS 1
#define ENEMY_TYPE_GNEISS 2
#define ENEMY_TYPE_BUTCHER 3

struct enemy_t {
    struct character_t base;
    float movement_speed;
    double animation_time;
    float hit_reset;
    float hit_recover_time;
    float time_to_attack;
    int being_hit;
    int engaging;
    int marked_for_delete;
    struct animation_t* walk;
    float walk_time;
    int walk_target_idx;
    float target_x;
    float target_z;
    float death_blink_time;
    int type;
    float damage_takes;
    float hit_streak;
};

struct player_t {
    struct character_t base;
    float movement_speed;
    float hit_recover_time;
    int score;
    float hit_reset;
    float hit_streak;
    struct animation_t* anim_upper_cut;
};

struct draw_t {
    HBITMAP hbitmap;
    int flip;
    COLORREF color_mask;
    struct vector3d_t position;
    int width;
    int height;
    int src_x;
    int src_y;
    int src_width;
    int src_height;
    int draw_top;
};

struct effect_t {
    struct sprite_t* sprite;
    struct vector3d_t pos;
    int draw_punch;
    struct animation_t* anim;
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

    float view_x;
    float view_x_far;

    float rect_width;

    float ground_y;
    float gravity;

    int state;
    char keyboard_state[256];

    struct player_t* player;

    struct enemy_t* enemies;
    int enemy_max;
    int enemy_count;

    HGDIOBJ bmp_bg;
    HGDIOBJ bmp_bg_far;
    HGDIOBJ bmp_level_layers[10];

    struct sprite_t* level_fire;

    HGDIOBJ bmp_avatar_player;

    struct effect_t fx_punch;
    struct effect_t fx_blood;
    HGDIOBJ bmp_text;
    float fx_smack_x;
    float fx_smack_y;
    float fx_smack_time;

    // things to draw on screen
    struct draw_t* draw_list;
    int draw_max;
    int draw_count;

    float max_z;
    float min_z;
    float max_view_x;
    float time_text_blink;

    float intro_time;
    int spawn_trigger;
};

extern struct game_t* game;

void vector3d_zero(struct vector3d_t* vec);

void graphics_clear();
void graphics_draw(struct draw_t* draw);
void graphics_draw_rect(HDC hdc, RECT* rect, COLORREF color);
void graphics_draw_surface(HDC hdc, HBITMAP bitmap, int x, int y, int w, int h,
                          int src_x, int src_y, int src_w, int src_h, int flip, COLORREF key);
void graphics_draw_lifebar(HDC hdc, int x, int y, int height, int width, float life);
void graphics_swap_buffer(HDC hdc, RECT* rect);

struct sprite_t* sprite_new(const wchar_t* filename, int width, int height, float scale);
void sprite_delete(struct sprite_t* sprite);
void sprite_update(struct sprite_t* sprite, float dt);
void sprite_draw(struct sprite_t* sprite, struct vector3d_t* pos, int direction, HDC hdc);
struct animation_t* sprite_add_animation(struct sprite_t* sprite, const wchar_t* name, int start, int end, int reverse_loop);
struct animation_t* sprite_set_animation(struct sprite_t* sprite, const wchar_t* name);
struct animation_t* sprite_current_animation(struct sprite_t* sprite);

struct player_t* player_new();
void player_delete(struct player_t* player);
void player_draw(struct player_t* player, HDC hdc);
void player_update(struct player_t* player, float dt);
void player_set_state(struct player_t* player, int state);
void character_get_center(struct character_t* character, struct vector3d_t* center);

struct enemy_t* enemy_spawn(int x, int y, int type);
void enemy_draw(struct enemy_t* enemy, HDC hdc);
void enemy_update(struct enemy_t* enemy, int index, float dt);
void enemy_cleanup(struct enemy_t* enemy);

void player_calculate_hit_boxes(struct player_t* player);
void enemy_calculate_hit_boxes(struct enemy_t* enemy) ;

float get_randf(float a, float b);
int get_rand(int min, int max);
HFONT create_font(const wchar_t* name, int size);

#endif

