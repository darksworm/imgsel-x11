#pragma once

#include <vector>
#include <string>
#include <glob.h>
#include <cstring>
#include <sstream>
#include <optional>
#include "../gui/WindowManager.h"

void drawText(WindowManager *windowManager, const std::string &text, Dimensions position);

void drawCenteredText(WindowManager *windowManager, const std::string &text, Dimensions position);

std::string getHomeDir();

struct CLIParams {
    unsigned int cacheSize;
    std::vector<std::string> imageFiles;
    bool startInVimMode = false;

    std::optional<unsigned int> maxImageWidth;
    std::optional<unsigned int> maxImageHeight;

    std::optional<unsigned int> rows;
    std::optional<unsigned int> cols;

    bool printFilePath = false;

    std::optional<unsigned int> imageXMargin;
    std::optional<unsigned int> imageYMargin;

    std::optional<unsigned int> imageXPadding;
    std::optional<unsigned int> imageYPadding;
};
