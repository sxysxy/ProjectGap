#include "stdafx.h"
#include "RGSSPlayer.h"

RGSSPlayer player;

int WINAPI WinMain(HINSTANCE hPrevInstance, HINSTANCE hInstance, LPSTR lpCmdLine, int nCmdShow) {
    __try {
        player.InitPlayer();
        player.MainLoop();
    }
    __except(EXCEPTION_EXECUTE_HANDLER){
        MessageBoxError(NULL, L"致命错误 Fatal Error", L"程序发生异常，已终止，请重新运行 \n Exception occured, Please restart");
        player.DestroyPlayer();
        ExitProcess(0);
       
    }
    player.DestroyPlayer();
    return 0;
}