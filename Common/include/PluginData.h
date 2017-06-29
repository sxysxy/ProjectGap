#pragma once
#include "GraphicsInfo.h"

struct PluginData {
    GraphicsInfo GraphicsInformation;
    HMODULE hRGSSCore;
    typedef unsigned(*pRGSSEval)(const char *lpScripts);
    pRGSSEval RGSSEval;
};