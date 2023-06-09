#include "game.h"

struct player_t* player_new() {
    struct player_t* player = malloc(sizeof(struct player_t));
    player->sprite = sprite_new(L"./player-walk.bmp", L"./player-walk-flip.bmp", 120, 90, 2.5);
    player->animation_time = 7.0f;
    player->movement_speed = 1.9;
    player->direction = CHARACTER_DIRECTION_RIGHT;
    player->state = CHARACTER_STATE_IDLE;
    vector3d_zero(&player->position);
    vector3d_zero(&player->velocity);

    player->position.x = 50;
    player->position.y = 280;

    player->hit_boxes = malloc(sizeof(RECT) * 10);
    player->hit_box_count = 0;

    sprite_add_animation(player->sprite, "idle", 0, 0, 0);
    sprite_add_animation(player->sprite, "walk", 1, 12, 0);
    sprite_add_animation(player->sprite, "punch", 13, 15, 1);

    player_set_state(player, CHARACTER_STATE_IDLE);

    return player;
};

void player_delete(struct player_t* player) {
    free(player->hit_boxes);
    sprite_delete(player->sprite);
    free(player);
}

void player_draw(struct player_t* player, HDC hdc) {
    sprite_draw(player->sprite, &player->position, player->direction, hdc);
}

void player_set_state(struct player_t* player, int state) {
    if (player->state != state) {
        switch(state) {
        case CHARACTER_STATE_IDLE:
            sprite_set_animation(player->sprite, "idle");
            break;
        case CHARACTER_STATE_WALKING:
            sprite_set_animation(player->sprite, "walk");
            break;
        case CHARACTER_STATE_FIGHT:
            sprite_set_animation(player->sprite, "punch");
            break;
        }
    }
    player->state = state;
}

void player_calculate_hit_boxes(struct player_t* player) {

    RECT* box = 0;
    player->hit_box_count = 0;
    struct vector3d_t* pos = &player->position;
    struct sprite_t* sprite = player->sprite;
    int state = player->state;
    int direction = player->direction;

    float scaled_width = sprite->width * sprite->scale;
    float scaled_height = sprite->height * sprite->scale;

    if (state == CHARACTER_STATE_IDLE || state == CHARACTER_STATE_WALKING) {
        box = &player->hit_boxes[player->hit_box_count];
        player->hit_box_count++;

        box->left = pos->x + (scaled_width/4);
        box->top = pos->y + pos->z;

        box->right = box->left + (scaled_width/2);
        box->bottom = box->top + scaled_height;
    } else if (state == CHARACTER_STATE_FIGHT) {
        // 1
        box = &player->hit_boxes[player->hit_box_count];
        player->hit_box_count++;

        box->left = pos->x + (scaled_width/4);
        box->top = pos->y + pos->z;

        box->right = box->left + (scaled_width/2);
        box->bottom = box->top + scaled_height;

        // 2
        box = &player->hit_boxes[player->hit_box_count];
        player->hit_box_count++;

        box->left = pos->x + (scaled_width/2);
        box->top = (pos->y + pos->z) + scaled_height / 7;

        if (sprite->frame == 13)
            box->right = box->left + (direction * (scaled_width/4));
        else if (sprite->frame == 14)
            box->right = box->left + (direction * (scaled_width/3));
        else if (sprite->frame == 15)
            box->right = box->left + (direction * (scaled_width/2));

        box->bottom = box->top + (scaled_height / 6);
    }
}

void player_update(struct player_t* player, float dt) {
    RECT rcClient;
    GetClientRect(game->hwnd, &rcClient);

    if (player->velocity.x > 0)
        player->direction = CHARACTER_DIRECTION_RIGHT;
    else if (player->velocity.x < 0)
        player->direction = CHARACTER_DIRECTION_LEFT;

    sprite_update(player->sprite, dt);

    struct vector3d_t old_pos = player->position;

    player->position.x += player->velocity.x * dt;
    player->position.y += player->velocity.y * dt;
    player->position.z += player->velocity.z * dt;

    if (player->position.z < -30 || player->position.z > 140) {
        player->position.z = old_pos.z;
    }

    player_calculate_hit_boxes(player);

    float scaled_width = (player->sprite->width * player->sprite->scale) / 2;
    float scaled_height = player->sprite->height * player->sprite->scale;

    if (player->state == CHARACTER_STATE_FIGHT) {
        //scaled_width = (player->sprite->width * player->sprite->scale) / 1.5;
        //player->collision_rect.left = player->position.x + (scaled_width/3);
    } else {

    }
}





