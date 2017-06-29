#include "stdafx.h"
#include "RGSSPlayer.h"

RGSSPlayer player;

int WINAPI WinMain(HINSTANCE hPrevInstance, HINSTANCE hInstance, LPSTR lpCmdLine, int nCmdShow) {
    player.InitPlayer();
    return 0;
}