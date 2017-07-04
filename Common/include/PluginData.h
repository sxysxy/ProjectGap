#pragma once
#include "GraphicsInfo.h"

struct PluginData {
    GraphicsInfo GraphicsInformation;
    HMODULE hRGSSCore;
    wchar_t *szRTPName;
    char RTPPath[MAX_PATH];
};