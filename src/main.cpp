#include <iostream>
#include <thread>
#include <X11/extensions/XTest.h>
#include <Imlib2.h>

#include "gui/WindowManager.h"
#include "gui/drawer/ShapeDrawerFactory.h"

#include "config/ConfigManager.h"
#include "util/ThreadSafeQueue.h"
#include "input/XEventWrapper.h"

#include <vector>
#include <string>
#include <sys/file.h>
#include <cerrno>
#include "input/InstructionRouter.h"
#include "input/XInputHandler.h"
#include "util/helpers.h"

int main(int argc, char *argv[]) {
    if (!XInitThreads()) {
        std::cout << "Failed to initialize XLib threads!";
        exit(1);
    }

    // TODO: disabled this temporarily as it is bugging the fuck out
    // TODO: is /tmp a good directory for a pid file?
//    int pid_file = open("/tmp/imgsel.pid", O_CREAT | O_RDWR, 0666);
//    int rc = flock(pid_file, LOCK_EX | LOCK_NB);
//    if (rc && EWOULDBLOCK == errno) {
//        std::cout << "another proccess is already running!";
//        exit(1);
//    }

    std::vector<std::string> imageFiles = glob(argv[1]);
    auto config = std::unique_ptr<Config>(ConfigManager::getOrLoadConfig());

    std::vector<std::string> imageExtensions = {
            "jpg",
            "jpeg",
            "png",
            "gif"
    };

    std::vector<Image> images;
    for (const auto &img:imageFiles) {
        for (const auto &ext:imageExtensions) {
            if (0 == img.compare(img.length() - ext.length(), ext.length(), ext)) {
                images.emplace_back(img);
                break;
            }
        }
    }

    // set up cache for images
    imlib_set_cache_size(config->getImageCacheSizeBytes());

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
