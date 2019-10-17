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
        config->maxImageHeight = maxImageWidth;
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
};
