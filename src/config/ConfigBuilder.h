#pragma once

#include "Config.h"

class ConfigBuilder {
private:
    Config* config;
public:
    ConfigBuilder() {
        config = new Config();
    }

    ConfigBuilder& setIsDebug(bool isDebug) {
        config->isDebug = isDebug;
        return *this;
    }
    
    ConfigBuilder& setImageCacheSizeBytes(unsigned int bytes) {
        config->imageCacheSizeBytes = bytes;
        return *this;
    }

    ConfigBuilder& setDefaultInputMode(InputMode mode) {
        config->defaultInputMode = mode;
        return *this;
    }

    Config* build() {
        return config;
    }

    ConfigBuilder& setMaxImageHeight(unsigned int maxImageHeight) {
        config->maxImageHeight = maxImageHeight;
        return *this;
    }

    ConfigBuilder& setMaxImageWidth(unsigned int maxImageWidth) {
        config->maxImageWidth = maxImageWidth;
        return *this;
    }

    ConfigBuilder& setRows(unsigned int rows) {
        config->rows = rows;
        return *this;
    }

    ConfigBuilder& setCols(unsigned int cols) {
        config->cols = cols;
        return *this;
    }

    ConfigBuilder& setPrintFilePath(bool printFilePath) {
        config->printFilePath = printFilePath;
        return *this;
    }

    ConfigBuilder& setYMargin(unsigned int yMargin) {
        config->yMargin = yMargin;
        return *this;
    }

    ConfigBuilder& setXMargin(unsigned int xMargin) {
        config->xMargin = xMargin;
        return *this;
    }

    ConfigBuilder& setYPadding(unsigned int yPadding) {
        config->yPadding = yPadding;
        return *this;
    }

    ConfigBuilder& setXPadding(unsigned int xPadding) {
        config->xPadding = xPadding;
        return *this;
    }
};
