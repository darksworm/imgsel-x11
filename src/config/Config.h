#pragma once

#include "../input/handler/InputMode.h"

class Config {
    friend class ConfigBuilder;

private:
    bool isDebug;
    unsigned int imageCacheSizeBytes;
    InputMode defaultInputMode = InputMode::DEFAULT;

    unsigned int maxImageWidth = 0;
    unsigned int maxImageHeight = 0;

    unsigned int rows = 0;
    unsigned int cols = 0;

    Config() = default;

public:
    bool isIsDebug() const {
        return isDebug;
    }

    unsigned int getImageCacheSizeBytes() {
        return imageCacheSizeBytes;
    }

    InputMode getDefaultInputMode() {
        return defaultInputMode;
    }

    unsigned int getMaxImageWidth() const {
        return maxImageWidth;
    }

    unsigned int getMaxImageHeight() const {
        return maxImageHeight;
    }

    unsigned int getRows() const {
        return rows;
    }

    unsigned int getCols() const {
        return cols;
    }
};