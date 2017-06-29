#pragma once
#include <windows.h>
#include <SDL2\include\SDL.h>

struct GraphicsInfo{
    HWND hWnd;
    SDL_Window *window;
    SDL_Renderer *renderer;
};
