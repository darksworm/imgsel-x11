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
#include "input/handler/instruction/CopyInstruction.h"
#include "util/ThreadSafeQueue.h"

#include <glob.h> // glob(), globfree()
#include <vector>
#include <stdexcept>
#include <string>
#include <sstream>
#include <sys/file.h>
#include <cerrno>

std::vector<std::string> glob(const std::string &pattern) {
    using namespace std;

    // glob struct resides on the stack
    glob_t glob_result;
    memset(&glob_result, 0, sizeof(glob_result));

    // do the glob operation
    int return_value = glob(pattern.c_str(), GLOB_TILDE, NULL, &glob_result);
    if (return_value != 0) {
        globfree(&glob_result);
        stringstream ss;
        ss << "glob() failed with return_value " << return_value << endl;
        throw std::runtime_error(ss.str());
    }

    // collect all the filenames into a std::list<std::string>
    vector<string> filenames;
    for (size_t i = 0; i < glob_result.gl_pathc; ++i) {
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

struct XEventWrapper {
    int eventType;
    KeySym keySym;
};

void handleXInput(WindowManager *windowManager, ThreadSafeQueue<XEventWrapper> *eventQueue, bool *shouldQuit) {
    auto display = windowManager->getDisplay();
    auto window = windowManager->getWindow();

    XSelectInput(display, window, ExposureMask | KeyPressMask | KeyReleaseMask | ButtonPressMask | StructureNotifyMask);
    XEvent event;

    bool keepRunning = true;

    while (keepRunning) {
        while (XPending(display)) {
            XNextEvent(display, &event);

            switch (event.type) {
                case DestroyNotify:
                case UnmapNotify:
                    eventQueue->push(XEventWrapper{
                            .eventType = -1
                    });
                    keepRunning = false;
                    break;

                case KeyRelease:
                case KeyPress: {
                    eventQueue->push(XEventWrapper{
                            .eventType = event.type,
                            .keySym = XLookupKeysym(&event.xkey, 0)
                    });
                    break;
                }

                default:
                    eventQueue->push(XEventWrapper{
                            .eventType = event.type
                    });
                    break;
            }
        }
        std::this_thread::sleep_for(std::chrono::nanoseconds(16600000));

        if (*shouldQuit) {
            keepRunning = false;
        }
    }
}

void handleEvents(WindowManager *windowManager, ThreadSafeQueue<XEventWrapper> *eventQueue,
                  std::vector<Image> images, bool *shouldQuit) {
    std::unique_ptr<ImagePickerDrawer> itemPickerDrawer(
            new ImagePickerDrawer(windowManager, &images)
    );

    itemPickerDrawer->drawFrame(&*images.begin());

    std::unique_ptr<InputHandler> inputHandler(nullptr);

    bool keep_running = true, readEvent;
    XEventWrapper event;
    InputMode state = InputMode::TEXT_FILTER;

    while (keep_running) {
        unsigned keyCode = 0;

        readEvent = eventQueue->next(event);

        /*
        * TODO: this probably shouldn't be a static value
        *   we should probably calculate how much time has passed between cycles
        *   and sleep for (16.6ms - passed time)
        */
        std::this_thread::sleep_for(std::chrono::nanoseconds(16600000));

        if (!readEvent) {
            continue;
        }

        if (event.eventType == -1) {
            keep_running = false;
            continue;
        }

        if (event.eventType == ConfigureNotify) {
            XClearWindow(windowManager->getDisplay(), windowManager->getWindow());
            auto hk = itemPickerDrawer->getSelectedImage() ? itemPickerDrawer->getSelectedImage()
                                                           : &*images.begin();
            itemPickerDrawer->drawFrame(hk);
            drawText(windowManager, "You're in TEXT_FILTER mode", Dimensions(900, 150));
            drawText(windowManager, "To change mode, press the CAPS LOCK key", Dimensions(900, 160));
            continue;
        }

        if (!InputHandlerFactory::isCorrectHandler(inputHandler.get(), state)) {
            inputHandler.reset(InputHandlerFactory::getInputHandler(state));
        }

        if (event.eventType == KeyPress || event.eventType == KeyRelease) {
            keyCode = x11_keycode_to_libinput_code(event.keySym);
        }

        printf("RAW: %s FORMATTED: %s %u\n", keycode_linux_rawname(keyCode),
               keycode_linux_name(keycode_linux_to_hid(keyCode)), keyCode);

        std::unique_ptr<Instruction> instruction;

        if (event.eventType == KeyPress) {
            instruction = std::unique_ptr<Instruction>(inputHandler->handleKeyPress(keyCode));
        } else if (event.eventType == KeyRelease) {
            instruction = std::unique_ptr<Instruction>(inputHandler->handleKeyRelease(keyCode));
        } else continue;

        if (instruction->getType() == InstructionType::NONE) {
            continue;
        }

        bool shouldExit =
                itemPickerDrawer->getFilterString().empty() && instruction->getType() == InstructionType::CANCEL;

        if (shouldExit || instruction->getType() == InstructionType::EXIT) {
            keep_running = false;
            continue;
        }

        if (instruction->getType() == InstructionType::CANCEL) {
            itemPickerDrawer->setFilter(nullptr, "");
            XClearWindow(windowManager->getDisplay(), windowManager->getWindow());
            itemPickerDrawer->drawFrame(nullptr);
        } else if (dynamic_cast<MoveInstruction *>(instruction.get())) {
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
            auto modeChangeInstruction = (ModeChangeInstruction *) instruction.get();
            state = modeChangeInstruction->newMode;

            if (modeChangeInstruction->shouldClearFilters) {
                itemPickerDrawer->setFilter(nullptr, "");
            }

            XClearWindow(windowManager->getDisplay(), windowManager->getWindow());
            itemPickerDrawer->drawFrame(nullptr);
        } else if (dynamic_cast<FilterInstruction *>(instruction.get())) {
            auto filterInstruction = ((FilterInstruction *) instruction.get());

            itemPickerDrawer->setFilter(filterInstruction->getFilter(), filterInstruction->getFilterString());
            XClearWindow(windowManager->getDisplay(), windowManager->getWindow());
            itemPickerDrawer->drawFrame(nullptr);
        } else if (dynamic_cast<CopyInstruction *>(instruction.get())) {
            auto selectedImage = itemPickerDrawer->getSelectedImage();
            auto path = selectedImage->getPath();
            auto ext = selectedImage->getExtension();

            ext = ext == "jpg" ? "jpeg" : ext;

            // TODO: handle 's in filenames
            std::string command = "cat '" + path + "' | xclip -selection clipboard -target image/" + ext + " -i";
            system(command.c_str());
            keep_running = false;
        }

        static const std::string inputModes[] = {"SELECTION", "TEXT_FILTER"};
        drawText(windowManager, "You're in " + inputModes[(int) state] + " mode", Dimensions(900, 150));
        drawText(windowManager, "To change mode, press the CAPS LOCK key", Dimensions(900, 160));

        if (!itemPickerDrawer->getFilterString().empty()) {
            drawText(windowManager, "QUERY: " + itemPickerDrawer->getFilterString(), Dimensions(500, 100));
        }
    }

    *shouldQuit = true;
}

int main(int argc, char *argv[]) {
    XInitThreads();

    // TODO: is /tmp a good directory for a pid file?
    int pid_file = open("/tmp/imgsel.pid", O_CREAT | O_RDWR, 0666);
    int rc = flock(pid_file, LOCK_EX | LOCK_NB);
    if (rc && EWOULDBLOCK == errno) {
        std::cout << "another proccess is already running!";
        exit(1);
    }

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

    auto windowManager = new WindowManager();

    Display *display = windowManager->getDisplay();
    windowManager->getWindow();

    XKeyboardState initKBState;
    XGetKeyboardControl(display, &initKBState);

    ThreadSafeQueue<XEventWrapper> eventQueue;
    bool shouldQuit;

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
