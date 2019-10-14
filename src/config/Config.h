#pragma once

#include "../input/handler/InputMode.h"

class Config {
    friend class ConfigBuilder;

private:
    bool isDebug;
    unsigned int imageCacheSizeBytes;
    InputMode defaultInputMode = InputMode::DEFAULT;
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
};