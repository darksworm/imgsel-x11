#pragma once

#include <vector>
#include <string>
#include <glob.h>
#include <cstring>
#include <sstream>
#include <optional>
#include "../gui/WindowManager.h"

void drawText(WindowManager *windowManager, const std::string &text, Dimensions position);

struct CLIParams {
    unsigned int cacheSize;
    std::vector<std::string> imageFiles;
    bool startInVimMode;

    std::optional<unsigned int> maxImageWidth;
    std::optional<unsigned int> maxImageHeight;

    std::optional<unsigned int> rows;
    std::optional<unsigned int> cols;
};
