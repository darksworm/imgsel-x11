#pragma once

#include "Config.h"
#include "../util/helpers.h"

class ConfigManager {
public:
    static Config getOrLoadConfig();
    ConfigManager();
    static void setCLIParams(CLIParams params);
private:
    inline static Config* config;
    inline static CLIParams cliParams;
    inline static bool configLoaded;
    inline static void loadConfig();
};