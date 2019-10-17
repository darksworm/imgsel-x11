#include <iostream>
#include <thread>
#include "lib/CLI11/include/CLI/CLI.hpp"

#include <X11/extensions/XTest.h>
#include <Imlib2.h>

#include "gui/WindowManager.h"
#include "gui/drawer/ShapeDrawerFactory.h"

#include "config/ConfigManager.h"
#include "util/ThreadSafeQueue.h"
#include "input/XEventWrapper.h"

#include <vector>
#include <string>
#include "input/InstructionRouter.h"
#include "input/XInputHandler.h"
#include "util/helpers.h"

int main(int argc, char *argv[]) {
    if (!XInitThreads()) {
        std::cout << "Failed to initialize XLib threads!";
        exit(1);
    }

    CLI::App app{"IMGSEL - Image selection tool."};

    CLIParams params = CLIParams();

    app.add_option("--files", params.imageFiles, "List of images to display.")
            ->required()
            ->check(CLI::ExistingFile);

    app.add_option("--cache-size", params.cacheSize, "How many (max) bytes of memory to use for caching loaded images.",
                   1024 * 1024 * 100);

    app.add_flag("--vim", params.startInVimMode, "Set the initial mode to VIM mode.");

    auto rowCountOption = app.add_option("--rows", params.rows, "How many rows to display");
    auto colCountOption = app.add_option("--cols", params.cols, "How many cols to display");

    rowCountOption->needs(colCountOption);
    colCountOption->needs(rowCountOption);

    auto widthOption = app.add_option("--max-width", params.maxImageWidth,
                                      "The max image width. Any images larger than this will be scaled to this width");
    auto heightOption = app.add_option("--max-height", params.maxImageHeight,
                                       "The max image height. Any images larger than this will be scaled to this height");

    heightOption->needs(widthOption);
    widthOption->needs(heightOption);

    CLI11_PARSE(app, argc, argv);

    ConfigManager::setCLIParams(params);
    auto config = ConfigManager::getOrLoadConfig();

    std::vector<std::string> imageExtensions = {
            "jpg",
            "jpeg",
            "png",
            "gif"
    };

    std::vector<Image> images;
    for (const auto &img:params.imageFiles) {
        for (const auto &ext:imageExtensions) {
            if (img.length() >= ext.length() && 0 == img.compare(img.length() - ext.length(), ext.length(), ext)) {
                images.emplace_back(img);
                break;
            }
        }
    }

    // set up cache for images
    imlib_set_cache_size(config.getImageCacheSizeBytes());

    auto windowManager = new WindowManager();

    Display *display = windowManager->getDisplay();
    windowManager->getWindow();

    XKeyboardState initKBState;
    XGetKeyboardControl(display, &initKBState);

    ThreadSafeQueue<XEventWrapper> eventQueue;
    bool shouldQuit = false;

    std::thread xEventListener(handleXInput, windowManager, &eventQueue, &shouldQuit);
    std::thread xEventHandler(handleEvents, windowManager, &eventQueue, images, &shouldQuit);

    xEventListener.join();
    xEventHandler.join();

    XKeyboardState exitKBState;
    XGetKeyboardControl(display, &exitKBState);

    if (exitKBState.led_mask & 1 != initKBState.led_mask & 1) {
        auto capsKeycode = XKeysymToKeycode(display, XK_Caps_Lock);

        // Simulate Press
        XTestFakeKeyEvent(display, capsKeycode, True, CurrentTime);
        XFlush(display);

        // Simulate Release
        XTestFakeKeyEvent(display, capsKeycode, False, CurrentTime);
        XFlush(display);
    }

    windowManager->destroyWindow();
    delete windowManager;
    return 0;
}
