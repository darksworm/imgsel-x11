#include <linux/input-event-codes.h>
#include "InputHandler.h"
#include "instruction/ModeChangeInstruction.h"
#include "../../lib/keycode/keycode.h"
#include "instruction/CopyInstruction.h"
#include "../../gui/ImagePickerDrawer.h"
#include "instruction/MoveInstruction.h"

Instruction *InputHandler::handleKeyPress(unsigned keyCode) {
    if (keyCode == KEY_ESC) {
        return new Instruction(InstructionType::CANCEL);
    }

    if (keyCode == KEY_CAPSLOCK) {
        InputMode mode = getNextMode();

        return new ModeChangeInstruction(mode);
    }

    if (keyCode == KEY_ENTER) {
        return new CopyInstruction();
    }

    if (isModifier(keyCode)) {
        addModifier(keyCode);
    }

    if (keyCode == KEY_C && isModifierActive("CONTROL")) {
        return new Instruction(InstructionType::EXIT);
    }

    ImagePickerMove move = ImagePickerMove::NONE;

    switch (keyCode) {
        case KEY_LEFT:
            move = ImagePickerMove::LEFT;
            break;
        case KEY_RIGHT:
            move = ImagePickerMove::RIGHT;
            break;
        case KEY_DOWN:
            move = ImagePickerMove::DOWN;
            break;
        case KEY_UP:
            move = ImagePickerMove::UP;
            break;
        case KEY_PAGEDOWN:
            move = ImagePickerMove::PG_DOWN;
            break;
        case KEY_PAGEUP:
            move = ImagePickerMove::PG_UP;
            break;
        case KEY_HOME:
            move = ImagePickerMove::HOME;
            break;
        case KEY_END:
            move = ImagePickerMove::END;
            break;
        default:
            move = ImagePickerMove::NONE;
            break;
    }

    if (move != ImagePickerMove::NONE) {
        return new MoveInstruction(move, 1);
    }

    return new Instruction(InstructionType::NONE);
}

Instruction *InputHandler::handleKeyRelease(unsigned keyCode) {
    if (isModifier(keyCode)) {
        removeModifier(keyCode);
    }

    return new Instruction(InstructionType::NONE);
}

void InputHandler::removeModifier(unsigned keyCode) {
    activeModifiers.erase(linux_keycode_to_yaml_name(keyCode));
}

bool InputHandler::isModifier(unsigned keyCode) {
    switch (keyCode) {
        case KEY_RIGHTSHIFT:
        case KEY_LEFTSHIFT:
            return true;
        case KEY_LEFTCTRL:
            return true;
    }

    return false;
}

void InputHandler::addModifier(unsigned keyCode) {
    activeModifiers.insert(linux_keycode_to_yaml_name(keyCode));
}

bool InputHandler::isModifierActive(std::string keyName) {
    return activeModifiers.find(keyName) != activeModifiers.end();
}
