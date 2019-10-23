#define DEBUG true

#include "ConfigManager.h"
#include "ConfigBuilder.h"

void ConfigManager::loadConfig() {
    ConfigManager::config = ConfigBuilder()
            .setIsDebug(DEBUG)
            .setImageCacheSizeBytes(cliParams.cacheSize)

            .setDefaultInputMode(cliParams.startInVimMode ? InputMode::VIM : InputMode::DEFAULT)

            .setCols(cliParams.cols.has_value() ? cliParams.cols.value() : 0)
            .setRows(cliParams.rows.has_value() ? cliParams.rows.value() : 0)

            .setMaxImageHeight(cliParams.maxImageHeight.has_value() ? cliParams.maxImageHeight.value() : 0)
            .setMaxImageWidth(cliParams.maxImageWidth.has_value() ? cliParams.maxImageWidth.value() : 0)

            .setPrintFilePath(cliParams.printFilePath)

            .setYPadding(cliParams.imageYPadding.has_value() ? cliParams.imageYPadding.value() : 30)
            .setXPadding(cliParams.imageXPadding.has_value() ? cliParams.imageXPadding.value() : 30)

            .setYMargin(cliParams.imageYMargin.has_value() ? cliParams.imageYMargin.value() : 30)
            .setXMargin(cliParams.imageXMargin.has_value() ? cliParams.imageXMargin.value() : 30)

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
