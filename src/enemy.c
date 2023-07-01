#include "game.h"

void enemy_cleanup(struct enemy_t* enemy) {
    free(enemy->base.hit_boxes);
    sprite_delete(enemy->base.sprite);
}

struct enemy_t* enemy_spawn(int x, int z, int type) {
    if (game->enemy_count >= game->enemy_max) {
        return 0;
    }
    struct enemy_t* enemy = &game->enemies[game->enemy_count];
    game->enemy_count++;

    enemy->type = type;
    enemy->base.health = 100.0f;

    enemy->animation_time = 0.0f;
    enemy->movement_speed = 2.5;
    enemy->base.direction = CHARACTER_DIRECTION_LEFT;
    enemy->base.prev_state = 0;
    enemy->base.visible = 1;
    enemy->hit_reset = 0.0;
    enemy->being_hit = 0;
    enemy->hit_recover_time = 0.0f;
    enemy->time_to_attack = 0;
    enemy->engaging = 0;
    enemy->marked_for_delete = 0;
    enemy->walk_time = 0;
    enemy->death_blink_time = 0;

    vector3d_zero(&enemy->base.position);
    vector3d_zero(&enemy->base.velocity);

    enemy->base.on_ground = 1;

    enemy->base.position.x = x;
    enemy->base.position.y = game->ground_y;
    enemy->base.position.z = z;

    enemy->base.hit_boxes = malloc(sizeof(RECT) * 10);
    enemy->base.hit_box_count = 0;

    enemy->damage_takes = 20;
    enemy->hit_streak = 0;

    struct animation_t* a;

    enemy->base.state_on_enter = 1;
    enemy->base.state = CHARACTER_STATE_IDLE;

    switch(type) {
    case ENEMY_TYPE_FERRIS:
        enemy->base.sprite = sprite_new(L"./enemy-1.bmp", 120, 90, 2.5);
        wcscpy(enemy->base.name, L"Ferris & Driver");
        a = sprite_add_animation(enemy->base.sprite, L"run", 6, 11, 0);
        a->speed = 0.9f;
        enemy->walk = sprite_add_animation(enemy->base.sprite, L"walk", 23, 29, 0);
        a = sprite_add_animation(enemy->base.sprite, L"punch", 12, 14, 1);
        a->speed = 1.3f;
        sprite_add_animation(enemy->base.sprite, L"punch_pre", 23, 23, 1);
        sprite_add_animation(enemy->base.sprite, L"hit", 21, 21, 1);
        a = sprite_add_animation(enemy->base.sprite, L"fall", 15, 19, 0);
        a->speed = 0.7f;
        a->play_once = 1;
        break;
    case ENEMY_TYPE_GNEISS:
        enemy->base.sprite = sprite_new(L"./enemy-2.bmp", 120, 90, 2.5);
        wcscpy(enemy->base.name, L"Gneiss");
        a = sprite_add_animation(enemy->base.sprite, L"run", 6, 11, 0);
        a->speed = 0.9f;
        enemy->walk = sprite_add_animation(enemy->base.sprite, L"walk", 23, 29, 0);
        a = sprite_add_animation(enemy->base.sprite, L"punch", 12, 14, 1);
        a->speed = 1.3f;
        sprite_add_animation(enemy->base.sprite, L"punch_pre", 23, 23, 1);
        sprite_add_animation(enemy->base.sprite, L"hit", 21, 21, 1);
        a = sprite_add_animation(enemy->base.sprite, L"fall", 15, 19, 0);
        a->speed = 0.7f;
        a->play_once = 1;
        break;
    case ENEMY_TYPE_BUTCHER:
        enemy->base.sprite = sprite_new(L"./enemy-butcher.bmp", 200, 130, 2.3);
        enemy->base.sprite->frames_per_row = 3;
        wcscpy(enemy->base.name, L"Butcher");
        enemy->damage_takes = 2.0f;
        enemy->movement_speed = 3.2;

        a = sprite_add_animation(enemy->base.sprite, L"run", 3, 8, 0);
        a->speed = 0.9f;
        enemy->walk = sprite_add_animation(enemy->base.sprite, L"walk", 3, 8, 0);
        a = sprite_add_animation(enemy->base.sprite, L"punch", 9, 11, 1);
        a->speed = 0.9f;
        sprite_add_animation(enemy->base.sprite, L"punch_pre", 0, 0, 1);
        sprite_add_animation(enemy->base.sprite, L"hit", 12, 12, 1);
        a = sprite_add_animation(enemy->base.sprite, L"fall", 12, 14, 0);
        a->speed = 0.3f;
        a->play_once = 1;

        //enemy->base.state = CHARACTER_STATE_RANGE_ATTACK;
        break;
    }

    a = sprite_add_animation(enemy->base.sprite, L"idle", 0, 2, 1);
    a->speed = 0.8f;

    sprite_add_animation(enemy->base.sprite, L"hit2", 20, 20, 1);
    sprite_add_animation(enemy->base.sprite, L"hit3", 22, 22, 1);


    enemy->walk_target_idx = -1;
    enemy->target_x = 0;
    enemy->target_z = 0;

    return enemy;
}

void enemy_draw(struct enemy_t* enemy, HDC hdc) {
    if (enemy->base.visible == 1) {
        sprite_draw(enemy->base.sprite, &enemy->base.position, enemy->base.direction, hdc);
    }
}

int game_random(int lower, int upper) {
    return (rand() % (upper - lower + 1)) + lower;
}

// this function does the whole enemy behavior
// Job: AI
void enemy_do_ai(struct enemy_t* enemy, float dt) {
    struct player_t* player = game->player;
    struct character_t* ch = &enemy->base;
    int type = enemy->type;
    struct vector3d_t player_center;
    struct vector3d_t enemy_center;
    float x, z;

    character_get_center(&player->base, &player_center);
    character_get_center(&enemy->base, &enemy_center);

    x = fabs(enemy_center.x - player_center.x);
    z = fabs(enemy_center.z - player_center.z);
    float player_distance_x = sqrtf(x * x);
    float player_distance_z = sqrtf(z * z);

    const float idle_range_x = 500;
    const float idle_range_z = 100;

    float fight_range_x = 150;
    const float fight_range_z = 10;

    if (type == ENEMY_TYPE_BUTCHER) {
        fight_range_x = 220;
    }

    if (game->state == GAME_STATE_INTRO) {
        ch->state = CHARACTER_STATE_IDLE;
    }

    // So when enemy is away from player and not being hit, there are two actions
    // 1. Run towards player to attack
    // 2. Move around and wait for attack

    if (enemy->being_hit) {
        enemy->being_hit = 0;

        struct animation_t* anim = sprite_current_animation(player->base.sprite);

        if (anim == player->anim_upper_cut) {
            // double damage
            ch->health -= enemy->damage_takes * 2;
            ch->state = CHARACTER_STATE_HIT_FALL;
            enemy->engaging = 0;
            enemy->hit_recover_time = 50;
            player->hit_streak = 0;

            ch->velocity.y = -10.0;
        } else {
            ch->health -= enemy->damage_takes + get_randf(0, 5);
            if (ch->health < 1) {
                ch->health = 0;
                ch->state = CHARACTER_STATE_HIT_FALL;
                enemy->engaging = 0;
                enemy->hit_recover_time = 70;
                player->hit_streak = 0;
                ch->velocity.y = -5.0;
            } else {
                ch->state = CHARACTER_STATE_HIT;
                if (enemy->hit_recover_time <= 0) {
                    enemy->hit_recover_time = 30;
                }
            }
        }
    }

    const float walk_time_wait = 100;

    // do stuffs based on current state
    if (ch->state == CHARACTER_STATE_IDLE) {
        struct animation_t* prev = sprite_current_animation(ch->sprite);
        struct animation_t* idle = sprite_set_animation(ch->sprite, L"idle");
        if (prev != idle) {
            ch->sprite->frame = game_random(idle->frame_start, idle->frame_end);
        }
        // while in idle state, enemy looks for player
        if (player->base.health > 0 && player_distance_x < idle_range_x && player_distance_z < idle_range_z) {
            float dir_x = player_center.x - enemy_center.x;

            if (dir_x > 0)
                ch->direction = CHARACTER_DIRECTION_RIGHT;
            else if (dir_x < 0)
                ch->direction = CHARACTER_DIRECTION_LEFT;
            // when butcher sees enemy first time
            /*if (type == ENEMY_TYPE_BUTCHER) {
                if (game->state == GAME_STATE_PLAYING)
                    game->state = GAME_STATE_CUTSCENE;
            } else */{
                // run for player to attack
                ch->state = CHARACTER_STATE_RUNNING;
            }
        }
        ch->velocity.x = 0.0;
        ch->velocity.z = 0.0;
    } else if (ch->state == CHARACTER_STATE_WALKING) {
        // walk around the player randomly (waiting to fight)

        float dir_x = player_center.x - enemy_center.x;

        if (dir_x > 0)
            ch->direction = CHARACTER_DIRECTION_RIGHT;
        else if (dir_x < 0)
            ch->direction = CHARACTER_DIRECTION_LEFT;

        //printf("dist %f\n", player_distance_x);

        sprite_set_animation(ch->sprite, L"walk");

        float target_distance_x = 0;
        float target_distance_z = 0;

        if (enemy->walk_target_idx >= 0) {
            // what if the target position is too close to another
            // enemy that they might overlap?
            // BUG: Enemy moves away afar
/*
            for(int i = 0;i < game->enemy_count;i++) {
                struct enemy_t* another_enemy = &game->enemies[i];
                if (another_enemy != enemy) {
                    if (another_enemy->walk_time == 0) {
                        struct vector3d_t another_enemy_center;
                        character_get_center(&another_enemy->base, &another_enemy_center);

                        float dx = fabs(enemy->target_x - another_enemy_center.x);
                        float dz = fabs(enemy->target_z - another_enemy_center.z);
                        float dist_x = sqrtf(dx * dx);
                        float dist_z = sqrtf(dz * dz);

                        if (dist_x < 100 && dist_z < 50) {

                            float dir = enemy->target_x - another_enemy_center.x;

                            if (dir != 0) {
                                float dist = sqrtf(dir * dir);
                                dir /= dist;

                                enemy->target_x += -dir * 50;
                                //enemy->target_z -= 2;
                            }
                            break;
                        }
                    }
                }
            }
            */

            if (enemy->target_z >= game->max_z) {
                enemy->target_z = game->max_z;
            }

            if (enemy->target_z <= game->min_z) {
                enemy->target_z = game->min_z;
            }

            float dx = fabs(enemy->target_x - enemy_center.x);
            float dz = fabs(enemy->target_z - enemy_center.z);
            target_distance_x = sqrtf(dx * dx);
            target_distance_z = sqrtf(dz * dz);
        }

        if (target_distance_x > 10 || target_distance_z > 10) {
            enemy->walk->play_once = 0;

            float dir_x = enemy->target_x - enemy_center.x;
            float dir_z = enemy->target_z - enemy_center.z;

            ch->velocity.x = 0.0;
            ch->velocity.z = 0.0;

            if (dir_x != 0) {
                float dist = sqrtf(dir_x * dir_x);
                dir_x /= dist;
                ch->velocity.x = dir_x * enemy->movement_speed * 1.5;
            }
            if (dir_z != 0) {
                float dist = sqrtf(dir_z * dir_z);
                dir_z /= dist;
                ch->velocity.z = dir_z * enemy->movement_speed * 1.5;
            }
        } else {
            enemy->walk->play_once = 1;
            ch->velocity.x = 0.0;
            ch->velocity.z = 0.0;

            if (enemy->walk_time < walk_time_wait) {
                enemy->walk_time += dt;
            } else {
                enemy->walk_time = 0;

                float x_list[4];
                float z_list[4];

                x_list[0] = player_center.x - 300;
                z_list[0] = player_center.z - 50;

                x_list[1] = player_center.x + 300;
                z_list[1] = player_center.z + 50;

                x_list[2] = player_center.x + 100;
                z_list[2] = player_center.z + 150;

                x_list[3] = player_center.x + 100;
                z_list[3] = player_center.z - 150;

                enemy->walk_target_idx = game_random(0, 3);

                enemy->target_x = x_list[enemy->walk_target_idx];
                enemy->target_z = z_list[enemy->walk_target_idx];

                if (enemy->target_z >= game->max_z) {
                    enemy->target_z = game->max_z;
                }

                if (enemy->target_z <= game->min_z) {
                    enemy->target_z = game->min_z;
                }
            }
        }

        // this enemy won't engage while any other enemy is engaging
        int can_fight = 1;
        for(int i = 0;i < game->enemy_count;i++) {
            struct enemy_t* another_enemy = &game->enemies[i];
            if (another_enemy != enemy) {
/*
                struct vector3d_t another_enemy_center;
                character_get_center(&another_enemy->base, &another_enemy_center);

                float dir_z = another_enemy_center.z - enemy_center.z;

                if (dir_z != 0) {
                    float dist = sqrtf(dir_z * dir_z);
                    dir_z /= dist;
                    ch->velocity.z = dir_z * enemy->movement_speed * 1.8;
                }
*/
                if (another_enemy->engaging) {
                    can_fight = 0;
                }
            }
        }
        if (can_fight) {
            ch->state = CHARACTER_STATE_PUNCH_PRE;
            enemy->engaging = 1;
            enemy->time_to_attack = 8;
        }
    } else if (ch->state == CHARACTER_STATE_MOVE) {
        sprite_set_animation(ch->sprite, L"walk");

        // move/walk to the target position

        float dir_x = enemy->target_x - enemy_center.x;
        float dir_z = enemy->target_z - enemy_center.z;
        float distance_x = sqrtf(dir_x * dir_x);
        float distance_z = sqrtf(dir_z * dir_z);

        if (distance_x > 2.0 ) {
            dir_x /= distance_x;
            ch->velocity.x = dir_x * enemy->movement_speed * 1.2;
        } else {
            ch->velocity.x = 0;
        }

        if (distance_z > 2.0 ) {
            dir_z /= distance_z;
            ch->velocity.z = dir_z * enemy->movement_speed * 1.2;
        } else {
            ch->velocity.z = 0;
        }

        if (fabs(distance_x) < 2.0 && fabs(distance_z) < 2.0) {
            ch->state = ch->prev_state;
            ch->prev_state = CHARACTER_STATE_MOVE;
        }

        // face towards the player? maybe
        float face_dir = player_center.x - enemy_center.x;

        if (face_dir > 0)
            ch->direction = CHARACTER_DIRECTION_RIGHT;
        else if (face_dir < 0)
            ch->direction = CHARACTER_DIRECTION_LEFT;

    } else if (ch->state == CHARACTER_STATE_RUNNING) {
        // run towards player
        float dir_x = player_center.x - enemy_center.x;
        float dir_z = player_center.z - enemy_center.z;

        if (player_distance_x < fight_range_x && player_distance_z < fight_range_z) {
            // this enemy won't engage while any other enemy is engaging
            int can_fight = 1;
            for(int i = 0;i < game->enemy_count;i++) {
                struct enemy_t* another_enemy = &game->enemies[i];
                if (another_enemy != enemy) {
                    if (another_enemy->engaging) {
                        can_fight = 0;
                        break;
                    }
                }
            }
            if (can_fight) {
                ch->state = CHARACTER_STATE_PUNCH_PRE;
                enemy->engaging = 1;
                enemy->time_to_attack = 8;
            } else {
                ch->state = CHARACTER_STATE_WALKING;
                enemy->walk_time = walk_time_wait;
            }
        } else {
            sprite_set_animation(ch->sprite, L"run");
            float dist_x = sqrtf(dir_x * dir_x);
            if (dist_x > 2.0) {
                dir_x /= dist_x;
                ch->velocity.x = dir_x * enemy->movement_speed;
            } else {
                ch->velocity.x = 0;
            }
            float dist_z = sqrtf(dir_z * dir_z);
            if (dist_z > 2.0) {
                dir_z /= dist_z;
                ch->velocity.z = dir_z * enemy->movement_speed;
            } else {
                ch->velocity.z = 0;
            }
        }

        if (ch->velocity.x > 0)
            ch->direction = CHARACTER_DIRECTION_RIGHT;
        else if (ch->velocity.x < 0)
            ch->direction = CHARACTER_DIRECTION_LEFT;

    } else if (ch->state == CHARACTER_STATE_PUNCH_PRE) {
        sprite_set_animation(ch->sprite, L"punch_pre");
        if (enemy->time_to_attack <= 0) {
            ch->state = CHARACTER_STATE_FIGHT;
        } else {
            enemy->time_to_attack -= dt * 0.5f;
        }
        ch->velocity.x = 0.0;
        ch->velocity.z = 0.0;
    } else if (ch->state == CHARACTER_STATE_RANGE_ATTACK) {
        sprite_set_animation(ch->sprite, L"idle");

        float dir_x = player_center.x - enemy_center.x;

        if (dir_x > 0)
            ch->direction = CHARACTER_DIRECTION_RIGHT;
        else if (dir_x < 0)
            ch->direction = CHARACTER_DIRECTION_LEFT;

        //printf("dist %f\n", player_distance_x);

        if (player_distance_x < 300) {
            // step back if too close
            enemy->target_x = enemy_center.x + (-ch->direction * 220);
            enemy->target_z = enemy_center.z;

            ch->prev_state = ch->state;
            ch->state = CHARACTER_STATE_MOVE;
            ch->state_on_enter = 1;
        }

        if (ch->state_on_enter == 1) {
            ch->state_on_enter = 0;
        } else {
            if (game->enemy_count == 1) {
                ch->prev_state = ch->state;
                ch->state = CHARACTER_STATE_RUNNING;
                ch->state_on_enter = 1;
            }
        }

    } else if (ch->state == CHARACTER_STATE_FIGHT) {
        sprite_set_animation(ch->sprite, L"punch");

        float dir_x = player_center.x - enemy_center.x;

        if (dir_x > 0)
            ch->direction = CHARACTER_DIRECTION_RIGHT;
        else if (dir_x < 0)
            ch->direction = CHARACTER_DIRECTION_LEFT;

        // what if player gets too close for the enemy to punch?
        if (player_distance_x < (fight_range_x / 1.2)) {
            // step back a little bit
            enemy->target_x = enemy_center.x + (-ch->direction * 50);
            enemy->target_z = enemy_center.z;

            ch->prev_state = ch->state;
            ch->state = CHARACTER_STATE_MOVE;
        }
        else if (player_distance_x > fight_range_x || player_distance_z > fight_range_z) {
            // run for player to attack
            ch->state = CHARACTER_STATE_RUNNING;
        } else {
            ch->velocity.x = 0.0;
            ch->velocity.z = 0.0;
        }

        if (player->base.health < 1) {
            ch->prev_state = ch->state;
            ch->state = CHARACTER_STATE_IDLE;
        }

    } else if (ch->state == CHARACTER_STATE_HIT) {
        sprite_set_animation(ch->sprite, L"hit");
        if (enemy->hit_recover_time <= 0) {
            // run for player to attack
            ch->state = CHARACTER_STATE_RUNNING;
        }
        enemy->hit_recover_time -= dt * 0.5f;

        ch->velocity.x = 0.0;
        ch->velocity.z = 0.0;

    } else if (ch->state == CHARACTER_STATE_HIT_FALL) {
        sprite_set_animation(ch->sprite, L"fall");

        int last_frame = 19;

        if (type == ENEMY_TYPE_BUTCHER) {
            last_frame = 14;
        }

        if (ch->sprite->frame >= last_frame) {
            if (ch->health > 0) {
                if (enemy->hit_recover_time <= 0) {
                    ch->prev_state = ch->state;
                    if (type == ENEMY_TYPE_BUTCHER) {
                        ch->state = CHARACTER_STATE_RANGE_ATTACK;
                        ch->state_on_enter = 1;

                        struct enemy_t* e;

                        if (ch->health <= 30) {
                            e = enemy_spawn(game->view_x + game->rect_width + 200, 0, ENEMY_TYPE_GNEISS);
                            e->base.state = CHARACTER_STATE_RUNNING;

                            e = enemy_spawn(game->view_x + game->rect_width + 100, 25, ENEMY_TYPE_GNEISS);
                            e->base.state = CHARACTER_STATE_RUNNING;

                            e = enemy_spawn(game->view_x + game->rect_width, 50, ENEMY_TYPE_FERRIS);
                            e->base.state = CHARACTER_STATE_RUNNING;

                            e = enemy_spawn(game->view_x - 200, 0, ENEMY_TYPE_GNEISS);
                            e->base.state = CHARACTER_STATE_RUNNING;

                            e = enemy_spawn(game->view_x - 100, 25, ENEMY_TYPE_GNEISS);
                            e->base.state = CHARACTER_STATE_RUNNING;

                            e = enemy_spawn(game->view_x , 50, ENEMY_TYPE_FERRIS);
                            e->base.state = CHARACTER_STATE_RUNNING;
                        } else if (ch->health <= 50) {
                            e = enemy_spawn(game->view_x + game->rect_width + 100, 0, ENEMY_TYPE_GNEISS);
                            e->base.state = CHARACTER_STATE_RUNNING;

                            e = enemy_spawn(game->view_x + game->rect_width, 50, ENEMY_TYPE_FERRIS);
                            e->base.state = CHARACTER_STATE_RUNNING;

                            e = enemy_spawn(game->view_x - 100, 00, ENEMY_TYPE_GNEISS);
                            e->base.state = CHARACTER_STATE_RUNNING;

                            e = enemy_spawn(game->view_x, 50, ENEMY_TYPE_FERRIS);
                            e->base.state = CHARACTER_STATE_RUNNING;
                        } else if (ch->health <= 80) {
                            e = enemy_spawn(game->view_x + game->rect_width, 0, ENEMY_TYPE_GNEISS);
                            e->base.state = CHARACTER_STATE_RUNNING;

                            e = enemy_spawn(game->view_x - 100, 50, ENEMY_TYPE_FERRIS);
                            e->base.state = CHARACTER_STATE_RUNNING;
                        }
                    } else {
                        ch->state = CHARACTER_STATE_RUNNING;
                    }
                } else {
                    enemy->hit_recover_time -= dt * 0.5f;
                }
            } else {
                if (enemy->hit_recover_time <= 0) {
                    enemy->marked_for_delete = 1;
                    if (enemy->type == ENEMY_TYPE_BUTCHER) {
                        game->state = GAME_STATE_WIN;
                        player_set_state(game->player, CHARACTER_STATE_WIN);
                    }
                } else {
                    enemy->hit_recover_time -= dt * 0.5f;

                    if (enemy->death_blink_time >= 8) {
                        enemy->death_blink_time = 0;
                        ch->visible = !ch->visible;
                    }
                    enemy->death_blink_time += dt;
                }
            }
        }

        if (type == ENEMY_TYPE_BUTCHER) {
            switch(ch->sprite->frame) {
            case 12:
                //printf("12\n");
                ch->velocity.x = -ch->direction * (enemy->movement_speed * 5.0);
                break;
            case 13:
                //printf("13\n");
                ch->velocity.x = -ch->direction * (enemy->movement_speed * 2.0);
                break;
            case 14:
                //printf("14\n");
                ch->velocity.x = 0.0;
                break;
            }
        } else {
            switch(ch->sprite->frame) {
            case 15:
                ch->velocity.x = -ch->direction * (enemy->movement_speed * 8.0);
                break;
            case 16:
                ch->velocity.x = -ch->direction * (enemy->movement_speed * 4.0);
                break;
            case 17:
                ch->velocity.x = -ch->direction * (enemy->movement_speed * 3.0);
                break;
            case 18:
                ch->velocity.x = -ch->direction * (enemy->movement_speed * 2.0);
                break;
            case 19:
                ch->velocity.x = 0.0;
                break;
            }
        }
        ch->velocity.z = 0.0;
    } else {
        ch->velocity.x = 0.0;
        ch->velocity.z = 0.0;
    }
}

void enemy_update(struct enemy_t* enemy, int index, float dt) {
    RECT rcClient;
    struct character_t* ch = &enemy->base;
    struct sprite_t* sprite = ch->sprite;

    GetClientRect(game->hwnd, &rcClient);

    struct vector3d_t old_pos = ch->position;

    sprite_update(sprite, dt);

    // this will only change the states, won't modify position or velocity
    enemy_do_ai(enemy, dt);

    ch->velocity.y += game->gravity * 0.3 * dt;

    ch->position.x += ch->velocity.x * dt;
    ch->position.y += ch->velocity.y * dt;
    ch->position.z += ch->velocity.z * dt;

    if (ch->position.y > game->ground_y) {

        if (ch->velocity.y > 2) {
            ch->velocity.y = -(ch->velocity.y * 0.6);
        } else {
            ch->position.y = game->ground_y;
            ch->velocity.y = 0;
            ch->on_ground = 1;
        }
    } else {
        ch->on_ground = 0;
    }

    enemy_calculate_hit_boxes(enemy);
}





