#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <windows.h>
#include "game.h"

struct game_t* game;

const float PI = 22.0f/7.0;

static LRESULT CALLBACK main_window_proc(HWND hwnd,
                        UINT message, WPARAM wParam, LPARAM lParam);

void sprite_sheet_draw(struct sprite_sheet_t* sheet,  int index) {

}


float get_randf(float a, float b) {
    float random = ((float) rand()) / (float) RAND_MAX;
    float diff = b - a;
    float r = random * diff;
    return a + r;
}

int get_rand(int min, int max) {
    return rand()%(max-min + 1) + min;
}

HFONT create_font(const wchar_t* name, int size) {
    HDC hDC = GetDC(HWND_DESKTOP);
    int hight = -MulDiv(size, GetDeviceCaps(hDC, LOGPIXELSY), 72);
    ReleaseDC(HWND_DESKTOP, hDC);
    HFONT font = CreateFontW(hight, 0, 0, 0, FW_BOLD, 0, 0, 0,
	   DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH, name);
    return font;
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
    game->height = 720;
    game->width = 1024;
    game->top_margin = 30;
    game->dbl_buffer = 0;

    const wchar_t window_class_name[] = L"main_window_class";

    wcscpy(game->title, L"Cardiac & Dianosours");

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
                    WS_OVERLAPPEDWINDOW,
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

    game->font = create_font(L"Arial", 26);
    game->font_big = create_font(L"Arial", 40);

    //game_load_resources();

    ShowWindow(game->hwnd, SW_SHOW);
    return 1;
}

int game_init() {
    game = (struct game_t*)malloc(sizeof(struct game_t));
    if (!game_init_window()) {
        return 0;
    }
    return 1;
}

void game_delete() {
    if (game != 0) {
            /*
        free(game->bullet_array);
        game_cleanup_enemies();
        game_cleanup_player();
        game_cleanup_resources();*/
        DeleteObject(game->font_big);
        DeleteObject(game->font);
        DeleteObject(game->dbl_buffer);
        free(game);
    }
}

void game_draw(HDC hdc, RECT* rect) {
}

void game_update(float dt) {
}

void game_exit() {
    PostQuitMessage(0);
}

void game_check_input() {
    if (game->state == GAME_STATE_PLAYING) {

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
        } else if (key == VK_SPACE) {

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


