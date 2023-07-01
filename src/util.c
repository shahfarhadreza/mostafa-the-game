#include "game.h"

void vector3d_zero(struct vector3d_t* vec) {
    vec->x = 0;
    vec->y = 0;
    vec->z = 0;
}

int get_rand(int min, int max) {
    return rand()%(max-min + 1) + min;
}

float get_randf(float a, float b) {
    float random = ((float) rand()) / (float) RAND_MAX;
    float diff = b - a;
    float r = random * diff;
    return a + r;
}

HFONT create_font(const wchar_t* name, int size) {
    HDC hDC = GetDC(HWND_DESKTOP);
    int hight = -MulDiv(size, GetDeviceCaps(hDC, LOGPIXELSY), 72);
    ReleaseDC(HWND_DESKTOP, hDC);
    HFONT font = CreateFontW(hight, 0, 0, 0, FW_BOLD, 0, 0, 0,
	   DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH, name);
    return font;
}

