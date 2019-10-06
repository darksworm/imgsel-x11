#include <memory>
#include <thread>
#include "../gui/WindowManager.h"
#include "../util/ThreadSafeQueue.h"
#include "XEventWrapper.h"
#include "../image/image.h"
#include "../gui/ImagePickerDrawer.h"
#include "handler/InputHandler.h"
#include "../lib/keycode/keycode.h"
#include "x11_keycodes.h"
#include "handler/InputHandlerFactory.h"
#include "handler/instruction/ModeChangeInstruction.h"
#include "handler/instruction/MoveInstruction.h"
#include "handler/instruction/FilterInstruction.h"
#include "handler/instruction/CopyInstruction.h"
#include "../util/helpers.h"
#include "InstructionRouter.h"

void handleEvents(WindowManager *windowManager, ThreadSafeQueue<XEventWrapper> *eventQueue,
                  std::vector<Image> images, bool *shouldQuit) {
    std::unique_ptr<ImagePickerDrawer> itemPickerDrawer(
            new ImagePickerDrawer(windowManager, &images)
    );

    std::unique_ptr<InputHandler> inputHandler(nullptr);

    bool keep_running = true, readEvent;
    XEventWrapper event;
    InputMode state = InputMode::TEXT_FILTER;

    bool initialFrameDrawn = false;

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
            if(initialFrameDrawn) {
                continue;
            }

            auto hk = itemPickerDrawer->getSelectedImage() ? itemPickerDrawer->getSelectedImage()
                                                           : &*images.begin();
            itemPickerDrawer->drawFrame(hk);
            drawText(windowManager, "You're in TEXT_FILTER mode", Dimensions(900, 150));
            drawText(windowManager, "To change mode, press the CAPS LOCK key", Dimensions(900, 160));

            initialFrameDrawn = true;
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
            itemPickerDrawer->drawFrame(nullptr);
        } else if (dynamic_cast<MoveInstruction *>(instruction.get())) {
            auto moveInstruction = ((MoveInstruction *) instruction.get());
            auto move = moveInstruction->getMoveDirection();

            bool moved = false;

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

            itemPickerDrawer->drawFrame(nullptr);
        } else if (dynamic_cast<FilterInstruction *>(instruction.get())) {
            auto filterInstruction = ((FilterInstruction *) instruction.get());

            itemPickerDrawer->setFilter(filterInstruction->getFilter(), filterInstruction->getFilterString());
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
        } else {
            // this should clear the previous query
            drawText(windowManager, "", Dimensions(500,100));
        }
    }

    *shouldQuit = true;
}


