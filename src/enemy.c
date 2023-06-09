#include "game.h"

void enemy_cleanup(struct enemy_t* enemy) {
    sprite_delete(enemy->sprite);
}

void enemy_remove(int index) {
    int count = game->enemy_count - 1;
    for(int i = index; i < count; i++) {
        game->enemies[i] = game->enemies[i + 1];
    }
    game->enemy_count--;
}

void enemy_spwan(int x, int y, int type) {
    if (game->enemy_count >= game->enemy_max) {
        return;
    }
    struct enemy_t* enemy = &game->enemies[game->enemy_count];
    game->enemy_count++;

    enemy->sprite = sprite_new(L"./enemy-1.bmp", L"./enemy-1-flip.bmp", 120, 90, 2.5);
    enemy->animation_time = 0.0f;
    enemy->movement_speed = 1.5;
    enemy->direction = CHARACTER_DIRECTION_LEFT;
    vector3d_zero(&enemy->position);
    vector3d_zero(&enemy->velocity);

    enemy->position.x = x;
    enemy->position.y = y;

    sprite_add_animation(enemy->sprite, "idle", 0, 2, 1);
    struct animation_t* a = sprite_add_animation(enemy->sprite, "run", 6, 11, 0);
    a->speed = 0.9f;
    //sprite_add_animation(enemy->sprite, "punch", 13, 15, 1);

    //player_set_state(player, CHARACTER_STATE_RUNNING);

    sprite_set_animation(enemy->sprite, "run");
    enemy->state = CHARACTER_STATE_RUNNING;

    enemy->velocity.x = -enemy->movement_speed;
}

void enemy_draw(struct enemy_t* enemy, HDC hdc) {
    sprite_draw(enemy->sprite, &enemy->position, enemy->direction, hdc);
}

void enemy_update(struct enemy_t* enemy, int index, float dt) {
    RECT rcClient;
    GetClientRect(game->hwnd, &rcClient);

    if (enemy->state == CHARACTER_STATE_RUNNING) {
        if (enemy->position.x < 0)
            enemy->velocity.x = enemy->movement_speed;
        if (enemy->position.x + enemy->sprite->width > rcClient.right) {
            enemy->velocity.x = -enemy->movement_speed;
        }
    }

    if (enemy->velocity.x > 0)
        enemy->direction = CHARACTER_DIRECTION_RIGHT;
    else if (enemy->velocity.x < 0)
        enemy->direction = CHARACTER_DIRECTION_LEFT;

    sprite_update(enemy->sprite, dt);

    enemy->position.x += enemy->velocity.x * dt;
    enemy->position.y += enemy->velocity.y * dt;
    enemy->position.z += enemy->velocity.z * dt;

    float scaled_width = (enemy->sprite->width * enemy->sprite->scale) / 2;
    float scaled_height = enemy->sprite->height * enemy->sprite->scale;

    if (enemy->state == CHARACTER_STATE_FIGHT) {
        scaled_width = (enemy->sprite->width * enemy->sprite->scale) / 1.5;
        enemy->collision_rect.left = enemy->position.x + (scaled_width/3);
    } else {
        enemy->collision_rect.left = enemy->position.x + (scaled_width/2);
    }


    enemy->collision_rect.top = enemy->position.y + enemy->position.z;

    enemy->collision_rect.right = enemy->collision_rect.left + scaled_width;
    enemy->collision_rect.bottom = enemy->collision_rect.top + scaled_height;
}





