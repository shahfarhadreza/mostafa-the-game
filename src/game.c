#include "game.h"

struct game_t* game;

const float PI = 22.0f/7.0;

static LRESULT CALLBACK main_window_proc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

void vector3d_zero(struct vector3d_t* vec) {
    vec->x = 0;
    vec->y = 0;
    vec->z = 0;
}

void game_init_dblbuffer() {
    RECT rcClient;
    GetClientRect(game->hwnd, &rcClient);
    if (game->dbl_buffer != 0) {
        DeleteObject(game->dbl_buffer);
    }
    const HDC hdc = GetDC(game->hwnd);
    game->dbl_buffer = CreateCompatibleBitmap(hdc, rcClient.right, rcClient.bottom);
    ReleaseDC(game->hwnd, hdc);
}

int game_init_window() {
    game->height = 668;
    game->width = 1024;
    game->top_margin = 30;
    game->dbl_buffer = 0;

    const wchar_t window_class_name[] = L"main_window_class";

    wcscpy(game->title, L"Cardiac & Dinosaurs");

    WNDCLASSEXW wincl;

    wincl.hInstance = GetModuleHandleW(0);
    wincl.lpszClassName = window_class_name;
    wincl.lpfnWndProc = main_window_proc;
    wincl.style = CS_DBLCLKS;
    wincl.cbSize = sizeof (WNDCLASSEX);

    wincl.hIcon = LoadIconW(0, IDI_APPLICATION);
    wincl.hIconSm = LoadIconW(0, IDI_APPLICATION);
    wincl.hCursor = LoadCursorW(0, IDC_ARROW);
    wincl.lpszMenuName = 0;
    wincl.cbClsExtra = 0;
    wincl.cbWndExtra = 0;
    wincl.hbrBackground = (HBRUSH) COLOR_BACKGROUND;

    if (RegisterClassExW(&wincl)) {
        game->hwnd = CreateWindowExW(
                    0,
                    window_class_name,
                    game->title,
                    WS_VISIBLE | WS_OVERLAPPED,
                    CW_USEDEFAULT,
                    CW_USEDEFAULT,
                    game->width,
                    game->height,
                    HWND_DESKTOP,
                    0,
                    wincl.hInstance,
                    0);
    } else {
        return 0;
    }

    game_init_dblbuffer();

    //game->font = create_font(L"Arial", 26);
    //game->font_big = create_font(L"Arial", 40);

    ShowWindow(game->hwnd, SW_SHOW);
    return 1;
}

void game_load_resources() {
    game->bmp_bg = LoadImageW(0, L"./ep-1.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
}

void graphics_clear() {
    game->draw_count = 0;
}
void graphics_draw(struct draw_t* args) {
    if (game->draw_count >= game->draw_max) {
        return;
    }

    struct draw_t* draw = &game->draw_list[game->draw_count];
    memcpy(draw, args, sizeof(struct draw_t));
    game->draw_count++;
}

int game_init() {
    game = (struct game_t*)malloc(sizeof(struct game_t));
    memset(game->keyboard_state, 0, sizeof(game->keyboard_state));

    if (!game_init_window()) {
        return 0;
    }

    game_load_resources();

    game->draw_max = 100;
    game->draw_list = malloc(sizeof(struct draw_t) * game->draw_max);
    game->draw_count = 0;

    game->state = GAME_STATE_PLAYING;

    game->player = player_new();

    game->enemy_count = 0;
    game->enemy_max = 100;

    game->enemies = malloc(sizeof(struct enemy_t) * game->enemy_max);

    //enemy_spwan(250, 280, 0);

    enemy_spwan(700, 280, 0);

    return 1;
}

void game_cleanup_enemies() {
    for(int i = 0;i < game->enemy_count;i++) {
        struct enemy_t* enemy = &game->enemies[i];
        enemy_cleanup(enemy);
    }
    free(game->enemies);
    game->enemy_count = 0;
}

void game_cleanup_resources() {
    DeleteObject(game->bmp_bg);
}

void game_delete() {
    if (game != 0) {
            /*
        free(game->bullet_array);*/
        game_cleanup_enemies();
        player_delete(game->player);
        free(game->draw_list);
        game_cleanup_resources();
        //DeleteObject(game->font_big);
        //DeleteObject(game->font);
        DeleteObject(game->dbl_buffer);
        free(game);
    }
}

void game_draw_background(HDC hdc, RECT* rect) {
    HDC hdc_bg = CreateCompatibleDC(hdc);
    SelectObject(hdc_bg, game->bmp_bg);

    StretchBlt(hdc, 0, 0, 2500, rect->bottom, hdc_bg, 0, 0, 928, 224, SRCCOPY);

    DeleteDC(hdc_bg);
}

int compare_draw( const struct draw_t* a, const struct draw_t* b) {
     if ( a->position.z == b->position.z )
        return 0;
     else if ( a->position.z < b->position.z )
        return -1;
     else
        return 1;
}

void game_debug(HDC hdc, RECT* rect) {
    HPEN pen = CreatePen(PS_SOLID, 2, RGB(255, 0, 0));
    HBRUSH brush = GetStockObject(NULL_BRUSH);
    SelectObject(hdc, pen);
    SelectObject(hdc, brush);
    Rectangle(hdc, rect->left, rect->top, rect->right, rect->bottom);
    DeleteObject(brush);
    DeleteObject(pen);
}

void game_draw_lifebar(HDC hdc, int height) {
    HBRUSH brush;
    HPEN pen;

    brush = CreateSolidBrush(RGB(255, 255, 0));

    SelectObject(hdc, brush);
    Rectangle(hdc, 10, 10, 50, 10 + height);

    DeleteObject(brush);

    pen = CreatePen(PS_SOLID, 2, RGB(255, 0, 0));
    brush = GetStockObject(NULL_BRUSH);
    SelectObject(hdc, pen);
    SelectObject(hdc, brush);
    Rectangle(hdc, 10, 10, 200, 10 + height);

    DeleteObject(brush);
    DeleteObject(pen);
}

void game_draw(HDC hdc, RECT* rect) {
    graphics_clear();

    // draw the moving background
    game_draw_background(hdc, rect);

    game_draw_lifebar(hdc, 30);

    for(int i = 0;i < game->enemy_count;i++) {
        struct enemy_t* enemy = &game->enemies[i];
        enemy_draw(enemy, hdc);
    }

    player_draw(game->player, hdc);

    // draw list (sorted by z/depth value of position)
    qsort(game->draw_list, game->draw_count, sizeof(struct draw_t), compare_draw);

    for(int i = 0;i < game->draw_count;i++) {
        struct draw_t* draw = &game->draw_list[i];

        HDC hdc_bmp = CreateCompatibleDC(hdc);
        SelectObject(hdc_bmp, draw->hbitmap);

        float x = draw->position.x;
        float y = draw->position.y + draw->position.z;

        TransparentBlt(hdc, x, y, draw->width, draw->height,
                       hdc_bmp, draw->src_x, draw->src_y,
                       draw->src_width, draw->src_height, RGB(0, 0, 248));
        DeleteDC(hdc_bmp);
    }

    for(int i = 0;i < game->enemy_count;i++) {
        struct enemy_t* enemy = &game->enemies[i];
        game_debug(hdc, &enemy->collision_rect);
    }

    for(int i = 0;i < game->player->hit_box_count;i++) {
        RECT* box = &game->player->hit_boxes[i];
        game_debug(hdc, box);
    }
}

void game_update(float dt) {
    player_update(game->player, dt);

    for(int i = 0;i < game->enemy_count;i++) {
        struct enemy_t* enemy = &game->enemies[i];
        enemy_update(enemy, i, dt);
    }
}

void game_exit() {
    PostQuitMessage(0);
}

void game_check_input() {
    if (game->state == GAME_STATE_PLAYING) {
        if (game->keyboard_state[VK_UP]) {
            //game->player->my = -game->player->movement_speed;
        } else if (game->keyboard_state[VK_DOWN]) {
            //game->player->my = game->player->movement_speed;
        } else {
            //game->player->my = 0.0f;
        }

        if (game->keyboard_state[VK_LEFT]) {
            game->player->velocity.x = -game->player->movement_speed;
            player_set_state(game->player, CHARACTER_STATE_WALKING);
        } else if (game->keyboard_state[VK_RIGHT]) {
            game->player->velocity.x = game->player->movement_speed;
            player_set_state(game->player, CHARACTER_STATE_WALKING);
        } else if (game->keyboard_state[VK_UP]) {
            game->player->velocity.z = -game->player->movement_speed;
            player_set_state(game->player, CHARACTER_STATE_WALKING);
        } else if (game->keyboard_state[VK_DOWN]) {
            game->player->velocity.z = game->player->movement_speed;
            player_set_state(game->player, CHARACTER_STATE_WALKING);
        } else if (game->keyboard_state[VK_SPACE]) {
            player_set_state(game->player, CHARACTER_STATE_FIGHT);
        } else {
            vector3d_zero(&game->player->velocity);
            player_set_state(game->player, CHARACTER_STATE_IDLE);
        }
    }
}

void game_input(int key, int down) {

    if (key == VK_ESCAPE) {
        game_exit();
    }

    if (down) {
        game->keyboard_state[key] = 1;
        if (key == VK_RETURN) {
            if (game->state == GAME_STATE_MENU)
                game->state = GAME_STATE_PLAYING;
            else if (game->state == GAME_STATE_PLAYING)
                game->state = GAME_STATE_PAUSED;
            else if (game->state == GAME_STATE_PAUSED)
                game->state = GAME_STATE_PLAYING;
        }
    } else {
        game->keyboard_state[key] = 0;
    }
}

LRESULT CALLBACK main_window_proc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
        case WM_KEYDOWN: {
            game_input(wParam, 1);
            break;
        }
        case WM_KEYUP: {
            game_input(wParam, 0);
            break;
        }
        case WM_ERASEBKGND: {
            return 1;
        }
        case WM_PAINT: {
            PAINTSTRUCT ps;
            BeginPaint(hwnd, &ps);
            const HDC hdc = ps.hdc;
            const HDC hdcBuffer = CreateCompatibleDC(hdc);
            SelectObject(hdcBuffer, game->dbl_buffer);

            RECT rcClient;
            GetClientRect(hwnd, &rcClient);

            HBRUSH page_bgk_brush = CreateSolidBrush(RGB(127, 184, 145));
            FillRect(hdcBuffer, &rcClient, page_bgk_brush);

            game_draw(hdcBuffer, &rcClient);

            BitBlt(hdc, 0,0, rcClient.right, rcClient.bottom, hdcBuffer, 0,0, SRCCOPY);

            DeleteDC(hdcBuffer);

            DeleteObject(page_bgk_brush);
            EndPaint(hwnd, &ps);
            break;
        }
        case WM_SIZE: {
            RECT rect;
            GetClientRect(hwnd, &rect);
            break;
        }
        case WM_DESTROY:
            PostQuitMessage(0);
            break;
        default:
            return DefWindowProcW(hwnd, message, wParam, lParam);
    }
    return 0;
}

void game_run() {
    MSG msg;
    LARGE_INTEGER start_counter, end_counter, frequency;
    double seconds_per_frame;
    QueryPerformanceCounter(&start_counter);
    QueryPerformanceFrequency(&frequency);
    while (1) {
        while(PeekMessage(&msg, 0, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        if(msg.message == WM_QUIT)
            break;
        QueryPerformanceCounter(&end_counter);

        seconds_per_frame = (((double)(end_counter.QuadPart -
                                       start_counter.QuadPart) * 100.0f) / (double)frequency.QuadPart);
        start_counter = end_counter;
        game_check_input();
        game_update(seconds_per_frame);
        InvalidateRect(game->hwnd, 0, 1);
	}
}

int main() {
    if (game_init()) {
        game_run();
        game_delete(game);
    }
    return 0;
}


