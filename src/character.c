#include "game.h"

void character_get_center(struct character_t* character, struct vector3d_t* center) {
    float scaled_width = character->sprite->width * character->sprite->scale;
    float scaled_height = character->sprite->height * character->sprite->scale;
    struct vector3d_t* pos = &character->position;

    center->x = pos->x + (scaled_width/2);
    center->y = pos->y + (scaled_height/2);
    center->z = pos->z;
}

int rect_collision(RECT* r1, RECT* r2) {
    if (r1->left < r2->right &&
        r1->right > r2->left &&
        r1->top < r2->bottom &&
        r1->bottom > r2->top)
    {
        return 1;
    }
    return 0;
}

void enemy_calculate_hit_boxes(struct enemy_t* enemy) {
    struct character_t* character = &enemy->base;
    RECT* box = 0;
    int type = enemy->type;
    struct vector3d_t* pos = &character->position;
    struct sprite_t* sprite = character->sprite;
    int state = character->state;
    int direction = character->direction;

    float scaled_width = sprite->width * sprite->scale;
    float scaled_height = sprite->height * sprite->scale;

    // Reset
    character->hit_box_count = 0;

    float center_x = pos->x + (scaled_width/2);

    // 1
    box = &character->hit_boxes[BBOX_MAIN];
    character->hit_box_count++;

    if (type == ENEMY_TYPE_BUTCHER) {
        box->left = center_x - (50 * sprite->scale);
        box->top = (pos->y + pos->z) - (120 * sprite->scale);

        box->right = center_x + (50 * sprite->scale);
        box->bottom = box->top + scaled_height;
    } else {
        box->left = pos->x + (scaled_width/4);
        box->top = pos->y + pos->z - scaled_height;

        box->right = box->left + (scaled_width/2);
        box->bottom = box->top + scaled_height;
    }

    float offset_head = 0;
    float offset_chest = 0;

    if (type == ENEMY_TYPE_BUTCHER) {
        offset_head = 90;
        offset_chest = 70;
    } else {
        offset_head = 70;
        offset_chest = 60;
    }

    if (state == CHARACTER_STATE_IDLE ||
        state == CHARACTER_STATE_WALKING ||
        state == CHARACTER_STATE_RUNNING ||
        state == CHARACTER_STATE_RANGE_ATTACK ||
        state == CHARACTER_STATE_HIT ||
        state == CHARACTER_STATE_MOVE ||
        state == CHARACTER_STATE_HIT_FALL) {
        // 2
        box = &character->hit_boxes[BBOX_HEAD];
        character->hit_box_count++;

        box->left = center_x - (16 * sprite->scale);
        box->top = (pos->y + pos->z) - (offset_head * sprite->scale);

        box->right = box->left + 15 * sprite->scale;
        box->bottom = box->top + (15 * sprite->scale);

        // 2
        box = &character->hit_boxes[BBOX_CHEST];
        character->hit_box_count++;

        box->left = pos->x + (scaled_width/2) - (12 * sprite->scale);
        box->top = (pos->y + pos->z) - (offset_chest * sprite->scale);

        box->right = pos->x + (scaled_width/2) + (12 * sprite->scale);
        box->bottom = box->top + (20 * sprite->scale);

    } else if (state == CHARACTER_STATE_FIGHT) {
        // 1

        box = &character->hit_boxes[BBOX_HEAD];
        character->hit_box_count++;

        box->left = center_x - (10 * sprite->scale);
        box->top = (pos->y + pos->z) - (offset_head * sprite->scale);

        box->right = box->left + 15 * sprite->scale;
        box->bottom = box->top + (15 * sprite->scale);

        // 2
        box = &character->hit_boxes[BBOX_CHEST];
        character->hit_box_count++;

        box->left = center_x - (20 * sprite->scale);
        box->top = (pos->y + pos->z) - (offset_chest * sprite->scale);

        box->right = box->left + (60 * sprite->scale);
        box->bottom = box->top + (40 * sprite->scale);

        // 3
        box = &character->hit_boxes[BBOX_FIST];
        character->hit_box_count++;

        float fistOffsetX = 0.0f;
        float fistOffsetY = 0.0f;

        int last_frame = 14;

        if (type == ENEMY_TYPE_BUTCHER) {
            last_frame = 11;

            if (sprite->frame == 9) {
                fistOffsetX = -100.0f;
                fistOffsetY = -50;
                enemy->hit_reset = 0;
            }
            else if (sprite->frame == 10) {
                fistOffsetX = -20.0f;
                fistOffsetY = 80;
            }
            else if (sprite->frame == 11) {
                fistOffsetX = 180.0f;
                fistOffsetY = 110;
            }
        } else {
            if (sprite->frame == 12) {
                fistOffsetX = -50.0f;
                enemy->hit_reset = 0;
            }
            else if (sprite->frame == 13)
                fistOffsetX = 40.0f;
            else if (sprite->frame == 14)
                fistOffsetX = 140.0f;
        }

        fistOffsetX *= -direction;

        box->left = center_x - (fistOffsetX + 10 * sprite->scale);
        box->top = pos->y + pos->z - scaled_height + (fistOffsetY + 21 * sprite->scale);

        box->right = box->left + (12 * sprite->scale);
        box->bottom = box->top + (12 * sprite->scale);

        struct effect_t* damagefx = &game->fx_punch;

        if (type == ENEMY_TYPE_BUTCHER) {
            damagefx = &game->fx_blood;
        }

        struct sprite_t* damage_sprite = damagefx->sprite;

        if (sprite->frame == last_frame) {
            float box_center_x = box->left + ((box->right - box->left) / 2);
            float box_center_y = box->top + ((box->bottom - box->top) / 2);

            float fx_half_w = (damage_sprite->width * damage_sprite->scale) / 2;
            float fx_half_h = (damage_sprite->height * damage_sprite->scale) / 2;
            damagefx->pos.x = (box_center_x - fx_half_w);
            damagefx->pos.y = box_center_y + fx_half_h;
            damagefx->pos.z = 0;

            if (type == ENEMY_TYPE_BUTCHER) {
                damagefx->pos.x -= 50;
                damagefx->pos.y += 10;
            }
        }

        struct player_t* player = game->player;
        RECT* player_head = &player->base.hit_boxes[BBOX_HEAD];
        RECT* player_chest = &player->base.hit_boxes[BBOX_CHEST];

        if (enemy->hit_reset == 0) {
            if ( rect_collision(box, player_chest) == 1 ||
                rect_collision(box, player_head) == 1) {
                if (player->base.health > 1) {
                    if (type == ENEMY_TYPE_BUTCHER) {
                        player->base.health -= 30;
                    } else {
                        player->base.health -= 15;
                    }

                    if ( player->base.health < 1) {
                        player_set_state(player, CHARACTER_STATE_HIT_FALL);
                    } else {
                        player_set_state(player, CHARACTER_STATE_HIT);
                    }

                    if (player->base.health < 0) {
                        player->base.health = 0;
                    }
                    damagefx->draw_punch = 1;
                    damage_sprite->frame = damagefx->anim->frame_start;
                    enemy->hit_reset = 1;
                }
            }
        }
    }
}

void player_calculate_hit_boxes(struct player_t* player) {
    struct character_t* character = &player->base;
    RECT* box = 0;
    struct vector3d_t* pos = &character->position;
    struct sprite_t* sprite = character->sprite;
    int state = character->state;
    int direction = character->direction;

    float scaled_width = sprite->width * sprite->scale;
    float scaled_height = sprite->height * sprite->scale;

    float center_x = pos->x + (scaled_width/2);

    // Reset
    character->hit_box_count = 0;

    if (state == CHARACTER_STATE_IDLE ||
        state == CHARACTER_STATE_WALKING ||
        state == CHARACTER_STATE_RUNNING ||
        state == CHARACTER_STATE_HIT_FALL) {

        box = &character->hit_boxes[BBOX_MAIN];
        character->hit_box_count++;

        box->left = pos->x + (scaled_width/4);
        box->top = pos->y + pos->z - scaled_height;

        box->right = box->left + (scaled_width/2);
        box->bottom = box->top + scaled_height;

        // 2
        box = &character->hit_boxes[BBOX_HEAD];
        character->hit_box_count++;

        box->left = pos->x + (scaled_width/2) - (8 * sprite->scale);
        box->top = (pos->y + pos->z) - (80 * sprite->scale);

        box->right = pos->x + (scaled_width/2) + 8 * sprite->scale;
        box->bottom = box->top + 15 * sprite->scale;

        // 3
        box = &character->hit_boxes[BBOX_CHEST];
        character->hit_box_count++;

        box->left = pos->x + (scaled_width/2) - (12 * sprite->scale);
        box->top = (pos->y + pos->z) - (70 * sprite->scale);

        box->right = pos->x + (scaled_width/2) + (12 * sprite->scale);
        box->bottom = box->top + (20 * sprite->scale);


    } else if (state == CHARACTER_STATE_FIGHT) {
        // 1
        box = &character->hit_boxes[BBOX_MAIN];
        character->hit_box_count++;

        box->left = pos->x + (scaled_width/4);
        box->top = pos->y + pos->z - scaled_height;

        box->right = box->left + (scaled_width/2);
        box->bottom = box->top + scaled_height;

        // 2
        box = &character->hit_boxes[BBOX_HEAD];
        character->hit_box_count++;

        box->left = pos->x + (scaled_width/2) - (8 * sprite->scale);
        box->top = (pos->y + pos->z) - (80 * sprite->scale);

        box->right = pos->x + (scaled_width/2) + 8 * sprite->scale;
        box->bottom = box->top + 15 * sprite->scale;

        // 3
        box = &character->hit_boxes[BBOX_CHEST];
        character->hit_box_count++;

        box->left = pos->x + (scaled_width/2) - (12 * sprite->scale);
        box->top = (pos->y + pos->z) - (70 * sprite->scale);

        box->right = pos->x + (scaled_width/2) + (12 * sprite->scale);
        box->bottom = box->top + (20 * sprite->scale);

        // 4
        box = &character->hit_boxes[BBOX_FIST];
        character->hit_box_count++;

        struct animation_t* anim = sprite_current_animation(sprite);

        float fistOffset = 0.0f;

        int last_frame = 15;

        if (anim == player->anim_upper_cut) {
            last_frame = 21;
            if (sprite->frame == 18) {
                fistOffset = 0.0f;
                player->hit_reset = 0;
            }
            else if (sprite->frame == 19)
                fistOffset = 50.0f;
            else if (sprite->frame == 20)
                fistOffset = 110.0f;
            else if (sprite->frame == 21)
                fistOffset = 150.0f;
        } else {
            if (sprite->frame == 13) {
                fistOffset = 0.0f;
                player->hit_reset = 0;
            }
            else if (sprite->frame == 14)
                fistOffset = 80.0f;
            else if (sprite->frame == 15)
                fistOffset = 140.0f;
        }

        fistOffset *= -direction;

        box->left = center_x - (fistOffset + 10 * sprite->scale);
        box->top = (pos->y + pos->z) - (70 * sprite->scale);

        box->right = box->left + (12 * sprite->scale);
        box->bottom = box->top + (12 * sprite->scale);

        struct effect_t* damagefx = &game->fx_punch;

        struct sprite_t* damage_sprite = damagefx->sprite;

        float box_center_x = box->left + ((box->right - box->left) / 2);
        float box_center_y = box->top + ((box->bottom - box->top) / 2);

        if (sprite->frame == last_frame) {

            float fx_half_w = (damage_sprite->width * damage_sprite->scale) / 2;
            float fx_half_h = (damage_sprite->height * damage_sprite->scale) / 2;
            damagefx->pos.x = box_center_x - fx_half_w;
            damagefx->pos.y = box_center_y + fx_half_h;
            damagefx->pos.z = 0;
        }

        if (player->hit_reset == 0 && sprite->frame == last_frame) {
            for (int i = 0;i < game->enemy_count;i++) {
                struct enemy_t* enemy = &game->enemies[i];
                if (enemy->base.health <= 0 || enemy->base.state == CHARACTER_STATE_HIT_FALL) {
                    continue;
                }
                RECT* enemy_head = &enemy->base.hit_boxes[BBOX_HEAD];
                RECT* enemy_chest = &enemy->base.hit_boxes[BBOX_CHEST];

                if ( rect_collision(box, enemy_chest) == 1 ||
                    rect_collision(box, enemy_head) == 1) {
                    enemy->being_hit = 1;
                    player->score += get_rand(50, 100);
                    player->hit_reset = 1;

                    player->hit_streak += 1;

                    damagefx->draw_punch = 1;
                    damage_sprite->frame = damagefx->anim->frame_start;

                    if (player->hit_streak > 2) {
                        game->fx_smack_time = 30;
                        game->fx_smack_x = box_center_x;
                        game->fx_smack_y = box_center_y;
                    }
                }
            }
        }
    }
}



