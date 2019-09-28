#include "FilteringInputHandler.h"
#include "../instruction/FilterInstruction.h"
#include "../instruction/ModeChangeInstruction.h"

Instruction *FilteringInputHandler::handleKeyPress(unsigned keyPress) {
    auto instruction = InputHandler::handleKeyPress(keyPress);

    switch (instruction->getType()) {
        case InstructionType::CANCEL: {
            if (!this->buffer.empty()) {
                this->buffer.clear();

                delete instruction;
                instruction = new FilterInstruction(this->getFilter(), this->getFilterText());
            }

            break;
        }

        case InstructionType::NONE: {
            if (shouldAddToBuffer(keyPress)) {
                this->buffer.push_back(keyPress);

                delete instruction;

                instruction = new FilterInstruction(this->getFilter(), this->getFilterText());
            }

            break;
        }
        default:
            break;
    }

    return instruction;
}

