#pragma once

#include <vector>
#include <string>
#include <glob.h>
#include <cstring>
#include <sstream>
#include "../gui/WindowManager.h"

std::vector<std::string> glob(const std::string &pattern);

void drawText(WindowManager *windowManager, const std::string &text, Dimensions position);

struct CLIParams {
    unsigned int cacheSize;
    std::vector<std::string> imageFiles;
};
