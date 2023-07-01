#include "game.h"

struct sprite_t* sprite_new(const wchar_t* filename, int width, int height, float scale) {
    struct sprite_t* sprite = malloc(sizeof(struct sprite_t));

    sprite->hbitmap = LoadImageW(0, filename, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    assert(sprite->hbitmap != 0);
    sprite->color_mask = RGB(0, 0, 248);
    sprite->width = width;
    sprite->height = height;
    sprite->scale = scale;
    sprite->draw_top = 0;
    sprite->frames_per_row = 6;
    sprite->frame = 0;
    sprite->frame_x = 0;
    sprite->frame_y = 0;
    sprite->flag = 0;
    sprite->animation_max = 10;
    sprite->animations = malloc(sizeof(struct animation_t) * sprite->animation_max);
    sprite->animation_time = 0.0f;
    sprite->animation_count = 0;
    sprite->animation_current = 0;
    return sprite;
}

void sprite_delete(struct sprite_t* sprite) {
    free(sprite->animations);
    DeleteObject(sprite->hbitmap);
    free(sprite);
}

struct animation_t* sprite_add_animation(struct sprite_t* sprite, const wchar_t* name, int start, int end, int reverse_loop) {
    if (sprite->animation_count >= sprite->animation_max) {
        return 0;
    }
    struct animation_t* anim = &sprite->animations[sprite->animation_count];
    wcscpy(anim->name, name);
    anim->frame_start = start;
    anim->frame_end = end;
    anim->speed = 1.0f;
    anim->reverse_loop = reverse_loop;
    anim->play_once = 0;
    anim->flag = 0;
    anim->animation_ended = 0;
    sprite->animation_current = sprite->animation_count;
    sprite->animation_count++;
    return anim;
}

struct animation_t* sprite_current_animation(struct sprite_t* sprite) {
    return &sprite->animations[sprite->animation_current];
}

struct animation_t* sprite_set_animation(struct sprite_t* sprite, const wchar_t* name) {
    assert(sprite->animation_current >= 0);
    //struct animation_t* canim = sprite_current_animation(sprite);

    for(int i = 0;i < sprite->animation_count;i++) {
        struct animation_t* anim = &sprite->animations[i];
        if(wcscmp(anim->name, name) == 0) {
            if (sprite->animation_current != i) {
                sprite->animation_current = i;
                sprite->frame = anim->frame_start;
                anim->flag = 0;
                anim->animation_ended = 0;
            }
            return anim;
        }
    }
    return 0;
}

void sprite_update(struct sprite_t* sprite, float dt) {
    if (sprite->animation_count) {
        struct animation_t* anim = &sprite->animations[sprite->animation_current];
        if (sprite->animation_time <= 0.0) {
            anim->animation_ended = 0;
            if (anim->reverse_loop) {
                if (anim->flag == 0) {
                    if (sprite->frame < anim->frame_end) {
                        sprite->frame += 1;
                    } else {
                        anim->flag = 1;
                    }
                } else {
                    if (sprite->frame > anim->frame_start) {
                        sprite->frame -= 1;
                    } else {
                        if (anim->play_once == 1) {
                            sprite->frame = anim->frame_start;
                            anim->animation_ended = 1;
                        } else {
                            anim->flag = 0;
                        }
                    }
                }
            } else {
                if (sprite->frame < anim->frame_end)
                    sprite->frame += 1;
                else {
                    if (anim->play_once == 0) {
                        sprite->frame = anim->frame_start;
                    } else {
                        sprite->frame = anim->frame_end;
                    }
                    anim->animation_ended = 1;
                }
            }
            sprite->animation_time = 8;
        }

        sprite->animation_time -= anim->speed * dt;
    }
}

void sprite_draw(struct sprite_t* sprite, struct vector3d_t* pos, int direction, HDC hdc) {
    int width = sprite->width;
    int height = sprite->height;

    sprite->frame_x = sprite->frame % sprite->frames_per_row;
    sprite->frame_y = floor(sprite->frame / sprite->frames_per_row);

    struct draw_t draw;

    if (direction == CHARACTER_DIRECTION_RIGHT)
        draw.flip = 0;
    else {
        draw.flip = 1;
    }
    draw.hbitmap = sprite->hbitmap;
    draw.position = *pos;
    draw.width = width*sprite->scale;
    draw.height = height*sprite->scale;
    draw.src_x = (sprite->frame_x * width);
    draw.src_y = (sprite->frame_y * height);
    draw.src_width = width;
    draw.src_height = height;
    draw.color_mask = sprite->color_mask;
    draw.draw_top = sprite->draw_top;

    graphics_draw(&draw);
}

void sprite_draw_2(struct sprite_t* sprite, struct draw_t* draw) {
    int width = sprite->width;
    int height = sprite->height;

    sprite->frame_x = sprite->frame % sprite->frames_per_row;
    sprite->frame_y = floor(sprite->frame / sprite->frames_per_row);

    draw->flip = 0;
    draw->hbitmap = sprite->hbitmap;
    draw->width = width*sprite->scale;
    draw->height = height*sprite->scale;

    draw->src_x = (sprite->frame_x * width);
    draw->src_y = (sprite->frame_y * height);
    draw->src_width = width;
    draw->src_height = height;
    draw->color_mask = sprite->color_mask;
    draw->draw_top = sprite->draw_top;
}



