

#include <linux/input-event-codes.h>
#include "SelectionInputHandler.h"
#include "../../gui/ImagePickerDrawer.h"
#include "instruction/MoveInstruction.h"
#include "instruction/ModeChangeInstruction.h"

InputInstruction *SelectionInputHandler::handleKeyPress(unsigned keyCode) {
    auto instruction = InputHandler::handleKeyPress(keyCode);

    if (keyCode == KEY_Q) {
        instruction = new InputInstruction(InputInstructionType::EXIT);
        return instruction;
    }

    if (instruction->getType() == InputInstructionType::NONE) {
        ImagePickerMove move = ImagePickerMove::NONE;

        switch (keyCode) {
            case KEY_SLASH:
                delete instruction;
                instruction = new ModeChangeInstruction(InputMode::DEFAULT);
                break;
            case KEY_H:
                move = ImagePickerMove::LEFT;
                break;
            case KEY_L:
                move = ImagePickerMove::RIGHT;
                break;
            case KEY_J:
                move = ImagePickerMove::DOWN;
                break;
            case KEY_K:
                move = ImagePickerMove::UP;
                break;
            default:
                move = ImagePickerMove::NONE;
                break;
        }

        if (move != ImagePickerMove::NONE) {
            delete instruction;
            instruction = new MoveInstruction(move, repeatNextCommandTimes);
        } else if (keyCode == KEY_C) {
            // change to the same mode, but clear filters.
            instruction = new ModeChangeInstruction(InputMode::VIM, 0, false, true);
        }
    }

    if (keyCode == KEY_0 && repeatNextCommand) {
        repeatNextCommandTimes = repeatNextCommandTimes * 10;
    } else if (keyCode >= KEY_1 && keyCode <= KEY_9) {
        if (repeatNextCommand) {
            repeatNextCommandTimes = repeatNextCommandTimes * 10 + (keyCode - 1);
        } else {
            repeatNextCommandTimes = keyCode - 1;
        }

        repeatNextCommand = true;
    } else if (!isModifier(keyCode) && keyCode != KEY_G) {
        repeatNextCommandTimes = 1;
        repeatNextCommand = false;
    }

    if (keyCode == KEY_G) {
        delete instruction;

        if (isModifierActive("SHIFT")) {
            if (repeatNextCommand) {
                unsigned targetLine = repeatNextCommandTimes;
                instruction = new MoveInstruction(ImagePickerMove::LINE, targetLine);
            } else {
                instruction = new MoveInstruction(ImagePickerMove::END, 1);
            }

        } else {
            instruction = new MoveInstruction(ImagePickerMove::HOME, 1);
        }

        repeatNextCommand = false;
        repeatNextCommandTimes = 1;
    }

    return instruction;
}

InputMode SelectionInputHandler::getNextMode() {
    return InputMode::DEFAULT;
}
