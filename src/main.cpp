#include <iostream>
#include <stdio.h>
#include <string.h>
#include <yaml-cpp/yaml.h>
#include <thread>
#include <X11/extensions/XTest.h>
#include <Imlib2.h>

#include "gui/WindowManager.h"
#include "lib/keycode/keycode.h"
#include "gui/drawer/ShapeDrawerFactory.h"

#include "input/x11_keycodes.h"
#include "input/handler/InputMode.h"
#include "input/handler/InputHandler.h"
#include "input/handler/InputHandlerFactory.h"
#include "input/handler/instruction/MoveInstruction.h"
#include "input/handler/instruction/ModeChangeInstruction.h"
#include "input/handler/instruction/FilterInstruction.h"
#include "config/ConfigManager.h"

#include <glob.h> // glob(), globfree()
#include <vector>
#include <stdexcept>
#include <string>
#include <sstream>

std::vector<std::string> glob(const std::string& pattern) {
    using namespace std;

    // glob struct resides on the stack
    glob_t glob_result;
    memset(&glob_result, 0, sizeof(glob_result));

    // do the glob operation
    int return_value = glob(pattern.c_str(), GLOB_TILDE, NULL, &glob_result);
    if(return_value != 0) {
        globfree(&glob_result);
        stringstream ss;
        ss << "glob() failed with return_value " << return_value << endl;
        throw std::runtime_error(ss.str());
    }

    // collect all the filenames into a std::list<std::string>
    vector<string> filenames;
    for(size_t i = 0; i < glob_result.gl_pathc; ++i) {
        filenames.push_back(string(glob_result.gl_pathv[i]));
    }

    // cleanup
    globfree(&glob_result);

    // done
    return filenames;
}

void drawText(WindowManager *windowManager, const std::string &text, Dimensions position) {
    XClearArea(
            windowManager->getDisplay(),
            windowManager->getWindow(),
            position.x,
            position.y,
            300,
            40,
            false
    );

    auto display = windowManager->getDisplay();
    auto window = windowManager->getWindow();

    // TODO: these should be config options
    int screen_num = DefaultScreen(display);

    GC gc = XCreateGC(display, window, 0, nullptr);

    XSetForeground(display, gc, WhitePixel(display, screen_num));
    XSetBackground(display, gc, BlackPixel(display, screen_num));

    XDrawString(
            windowManager->getDisplay(),
            windowManager->getWindow(),
            gc,
            (int) position.x,
            (int) position.y,
            text.c_str(),
            (int) text.length()
    );

    XFreeGC(display, gc);
}

int main(int argc, char *argv[]) {
    std::vector<std::string> imageFiles = glob(argv[1]);
    auto config = std::unique_ptr<Config>(ConfigManager::getOrLoadConfig());

    std::vector<std::string> imageExtensions = {
        "jpg",
        "jpeg",
        "png",
        "gif"
    };

    std::vector<Image> images;
    for(const auto& img:imageFiles) {
        for(const auto& ext:imageExtensions) {
            if (0 == img.compare(img.length() - ext.length(), ext.length(), ext)) {
                images.emplace_back(img);
                break;
            }
        }
    }

    std::unique_ptr<WindowManager> windowManager(new WindowManager());
    std::unique_ptr<ImagePickerDrawer> itemPickerDrawer(
            new ImagePickerDrawer(windowManager.get(), &images)
    );

    Display *display = windowManager->getDisplay();
    Window window = windowManager->getWindow();

    XKeyboardState initKBState;
    XGetKeyboardControl(display, &initKBState);

    InputMode state = InputMode::SELECTION;
    int keep_running = 1;
    XEvent event;

    XSelectInput(display, window, ExposureMask | KeyPressMask | KeyReleaseMask | ButtonPressMask | StructureNotifyMask);
    itemPickerDrawer->drawFrame(&*images.begin());

    std::unique_ptr<InputHandler> inputHandler(nullptr);

    unsigned eventType = 0;

    while (keep_running) {
        unsigned keyCode = 0;

        // read xevents
        while (XPending(display)) {
            XNextEvent(display, &event);

            switch (event.type) {
                case DestroyNotify:
                case UnmapNotify:
                    keep_running = 0;
                    break;

                case KeyPress: {
                    eventType = KeyPress;
                    keyCode = x11_keycode_to_libinput_code(XLookupKeysym(&event.xkey, 0));
                    break;
                }

                case KeyRelease:
                    eventType = KeyRelease;
                    keyCode = x11_keycode_to_libinput_code(XLookupKeysym(&event.xkey, 0));
                    break;

                case ConfigureNotify:
                    XClearWindow(windowManager->getDisplay(), windowManager->getWindow());
                    auto hk = itemPickerDrawer->getSelectedImage() ? itemPickerDrawer->getSelectedImage()
                                                                    : &*images.begin();
                    itemPickerDrawer->drawFrame(hk);
                    break;
            }
        }

        /*
        * TODO: this probably shouldn't be a static value
        *   we should probably calculate how much time has passed between cycles
        *   and sleep for (16.6ms - passed time)
        */
        std::this_thread::sleep_for(std::chrono::nanoseconds(16600000));


        if (keyCode == 0) {
            continue;
        }

        if (!InputHandlerFactory::isCorrectHandler(inputHandler.get(), state)) {
            inputHandler.reset(InputHandlerFactory::getInputHandler(state));
        }

        printf("RAW: %s FORMATTED: %s %u\n", keycode_linux_rawname(keyCode),
               keycode_linux_name(keycode_linux_to_hid(keyCode)), keyCode);

        std::unique_ptr<Instruction> instruction;

        if (eventType == KeyPress) {
            instruction = std::unique_ptr<Instruction>(inputHandler->handleKeyPress(keyCode));
        } else {
            instruction = std::unique_ptr<Instruction>(inputHandler->handleKeyRelease(keyCode));
        }

        if (instruction->getType() == InstructionType::NONE) {
            continue;
        }

        if (instruction->getType() == InstructionType::EXIT) {
            keep_running = 0;
            continue;
        }

        if (dynamic_cast<MoveInstruction *>(instruction.get())) {
            auto moveInstruction = ((MoveInstruction *) instruction.get());
            auto move = moveInstruction->getMoveDirection();

            bool moved = false;

            XClearWindow(windowManager->getDisplay(), windowManager->getWindow());

            if (move != ImagePickerMove::NONE) {
                moved = itemPickerDrawer->move(moveInstruction->getMoveDirection(), moveInstruction->getMoveSteps());
            }

            if (move == ImagePickerMove::NONE || !moved) {
                itemPickerDrawer->drawFrame(itemPickerDrawer->getSelectedImage());
            }
        } else if (dynamic_cast<ModeChangeInstruction *>(instruction.get())) {
            state = ((ModeChangeInstruction *) (instruction.get()))->getNewMode();

            itemPickerDrawer->setFilter(nullptr);
            XClearWindow(windowManager->getDisplay(), windowManager->getWindow());
            itemPickerDrawer->drawFrame(nullptr);
        } else if (dynamic_cast<FilterInstruction *>(instruction.get())) {
            auto filterInstruction = ((FilterInstruction *) instruction.get());

            itemPickerDrawer->setFilter(filterInstruction->getFilter());
            XClearWindow(windowManager->getDisplay(), windowManager->getWindow());
            itemPickerDrawer->drawFrame(nullptr);

            if (config->isIsDebug()) {
                drawText(windowManager.get(), "QUERY: " + filterInstruction->getFilterString(), Dimensions(500, 100));
            }
        }

        if (config->isIsDebug()) {
            static const char *inputModes[] = {"SELECTION", "TEXT_FILTER"};
            drawText(windowManager.get(), inputModes[(int) state], Dimensions(100, 100));
        }
    }

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

    return 0;
}
