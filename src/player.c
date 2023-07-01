#include "game.h"

struct player_t* player_new() {
    struct player_t* player = malloc(sizeof(struct player_t));
    player->base.sprite = sprite_new(L"./player-jack.bmp", 120, 110, 2.5);
    player->hit_recover_time = 0.0f;
    player->movement_speed = 2.7;
    player->score = 0;
    player->base.direction = CHARACTER_DIRECTION_RIGHT;
    player->base.state = 0;
    player->base.prev_state = 0;
    player->base.health = 100.0f;
    player->base.visible = 1;
    player->base.on_ground = 1;
    vector3d_zero(&player->base.position);
    vector3d_zero(&player->base.velocity);

    player->hit_reset = 0;
    player->hit_streak = 0;

    player->base.position.x = 350;
    player->base.position.y = game->ground_y;

    player->base.hit_boxes = malloc(sizeof(RECT) * 10);
    player->base.hit_box_count = 0;

    struct animation_t* a;

    sprite_add_animation(player->base.sprite, L"idle", 0, 0, 0);
    a = sprite_add_animation(player->base.sprite, L"walk", 1, 12, 0);
    a->speed = 1.2f;
    a = sprite_add_animation(player->base.sprite, L"punch", 13, 15, 1);
    a->speed = 3.5f;
    a->play_once = 1;

    player->anim_upper_cut = sprite_add_animation(player->base.sprite, L"punch2", 18, 21, 1);
    player->anim_upper_cut->speed = 1.5f;
    player->anim_upper_cut->play_once = 1;

    sprite_add_animation(player->base.sprite, L"hit", 16, 16, 0);

    a = sprite_add_animation(player->base.sprite, L"fall", 30, 35, 0);
    a->speed = 0.5f;
    a->play_once = 1;

    a = sprite_add_animation(player->base.sprite, L"jump", 24, 26, 1);
    a->speed = 1.3f;
    a->play_once = 1;

    a = sprite_add_animation(player->base.sprite, L"win", 27, 29, 0);
    a->speed = 0.5f;
    a->play_once = 1;

    player_set_state(player, CHARACTER_STATE_IDLE);

    return player;
};

void player_delete(struct player_t* player) {
    free(player->base.hit_boxes);
    sprite_delete(player->base.sprite);
    free(player);
}

void player_draw(struct player_t* player, HDC hdc) {
    if (player->base.visible == 1) {
        sprite_draw(player->base.sprite, &player->base.position, player->base.direction, hdc);
    }
}

void player_set_state(struct player_t* player, int state) {
    struct character_t* ch = &player->base;
    struct sprite_t* sprite = ch->sprite;

    // let us finish the prev punch animation
    if (ch->state == CHARACTER_STATE_FIGHT) {
        struct animation_t* anim = sprite_current_animation(sprite);
        if (!anim->animation_ended) {
            return;
        }
    }

    if (player->base.prev_state == CHARACTER_STATE_HIT_FALL) {
        return;
    }

    switch(state) {
    case CHARACTER_STATE_IDLE:
        sprite_set_animation(player->base.sprite, L"idle");
        break;
    case CHARACTER_STATE_WALKING:
        sprite_set_animation(player->base.sprite, L"walk");
        break;
    case CHARACTER_STATE_JUMP:
        sprite_set_animation(player->base.sprite, L"jump");
        if (ch->on_ground) {
            ch->velocity.y = -18.0f;
        }
        break;
    case CHARACTER_STATE_FIGHT:
        // if we are about to punch 3rd time in a row
        if (player->hit_streak >= 2) {
            sprite_set_animation(player->base.sprite, L"punch2");
        } else {
            sprite_set_animation(player->base.sprite, L"punch");
        }
        break;
    case CHARACTER_STATE_HIT:
        sprite_set_animation(player->base.sprite, L"hit");
        if (player->base.state != state) {
            player->hit_recover_time = 10;
        }
        break;
    case CHARACTER_STATE_HIT_FALL:
        sprite_set_animation(player->base.sprite, L"fall");
        if (player->base.state != state) {
            player->hit_recover_time = 300;
        }
        break;
    case CHARACTER_STATE_WIN:
        sprite_set_animation(player->base.sprite, L"win");
        break;
    }
    if (player->base.state != state) {
        player->base.prev_state = player->base.state;
        player->base.state = state;
    }
}

void player_update_state(struct player_t* player, float dt) {
    struct character_t* ch = &player->base;
    struct sprite_t* sprite = ch->sprite;

    if (ch->state != CHARACTER_STATE_HIT_FALL) {
        if ( ch->health < 1) {
            player_set_state(player, CHARACTER_STATE_HIT_FALL);
        }
    }

    if (ch->state == CHARACTER_STATE_FIGHT) {
        struct animation_t* anim = sprite_current_animation(sprite);
        if (anim->animation_ended) {
            player_set_state(player, CHARACTER_STATE_IDLE);
        }
    } else if (ch->state == CHARACTER_STATE_HIT) {
        if (player->hit_recover_time <= 0) {
            player_set_state(player, CHARACTER_STATE_IDLE);
        }
        player->hit_recover_time -= dt;
    } else if (ch->state == CHARACTER_STATE_HIT_FALL) {
        if (player->hit_recover_time <= 0) {
            game->state = GAME_STATE_OVER;
        } else {
            player->hit_recover_time -= dt;
        }
    } else if (ch->state == CHARACTER_STATE_JUMP) {
        struct animation_t* anim = sprite_current_animation(sprite);
/*
        printf("frame %d\n", sprite->frame);
        switch(sprite->frame) {
        case 24:
            ch->velocity.y = -5;
            break;
        case 25:
            //ch->velocity.y = 5;
            break;
        case 26:
            ch->velocity.y += 5;
            break;
        }
*/
        if (anim->animation_ended) {
            player_set_state(player, CHARACTER_STATE_IDLE);
            ch->velocity.y = 0;
        }
    }
}

void player_update(struct player_t* player, float dt) {
    RECT rcClient;
    GetClientRect(game->hwnd, &rcClient);
    struct character_t* ch = &player->base;

    if (ch->velocity.x > 0)
        ch->direction = CHARACTER_DIRECTION_RIGHT;
    else if (ch->velocity.x < 0)
        ch->direction = CHARACTER_DIRECTION_LEFT;

    sprite_update(ch->sprite, dt);

    player_update_state(player, dt);

    if (game->state == GAME_STATE_PLAYING) {
        struct vector3d_t old_pos = ch->position;

        ch->velocity.y += game->gravity * dt;

        ch->position.x += ch->velocity.x * dt;
        ch->position.y += ch->velocity.y * dt;
        ch->position.z += ch->velocity.z * dt;

        if (ch->position.z < game->min_z || ch->position.z > game->max_z) {
            ch->position.z = old_pos.z;
        }

        if (ch->position.y >= game->ground_y) {
            ch->position.y = game->ground_y;
            ch->velocity.y = 0;
            ch->on_ground = 1;
        } else {
            ch->on_ground = 0;
        }


        player_calculate_hit_boxes(player);

        if (ch->position.x >= 400) {
            game->view_x = ch->position.x - 400;
        }

        if (ch->position.x <= 150) {
            game->view_x = ch->position.x - 150;
        }

        if (game->view_x > game->max_view_x) {
            game->view_x = game->max_view_x;
        }

        if (game->view_x < 0) {
            game->view_x = 0;
        }
        game->view_x_far = game->view_x * 0.6f;

    }
}





