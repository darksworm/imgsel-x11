#define DEBUG true

#include "ConfigManager.h"
#include "ConfigBuilder.h"

void ConfigManager::loadConfig() {
    ConfigManager::config = ConfigBuilder()
            .setIsDebug(DEBUG)
            .setImageCacheSizeBytes(cliParams.cacheSize)
            .setDefaultInputMode(cliParams.startInVimMode ? InputMode::VIM : InputMode::DEFAULT)
            .build();
}

Config ConfigManager::getOrLoadConfig() {
    if (!ConfigManager::configLoaded) {
        loadConfig();
    }

    return *ConfigManager::config;
}

ConfigManager::ConfigManager() {
    ConfigManager::configLoaded = false;
}

void ConfigManager::setCLIParams(CLIParams params) {
    ConfigManager::cliParams = std::move(params);
}
