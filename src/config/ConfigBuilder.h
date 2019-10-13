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

    Config* build() {
        return config;
    }
};
