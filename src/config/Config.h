#pragma once

class Config {
    friend class ConfigBuilder;

private:
    bool isDebug;
    unsigned int imageCacheSizeBytes;
    Config() = default;
public:
    bool isIsDebug() const {
        return isDebug;
    }

    unsigned int getImageCacheSizeBytes() {
        return imageCacheSizeBytes;
    }
};