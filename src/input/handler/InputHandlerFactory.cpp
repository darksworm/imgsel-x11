

#include "InputHandlerFactory.h"
#include "SelectionInputHandler.h"
#include "filters/TextFilteringInputHandler.h"

InputHandler *InputHandlerFactory::getInputHandler(InputMode type) {
    switch (type) {
        case InputMode::TEXT_FILTER:
            return new TextFilteringInputHandler();
        case InputMode::SELECTION:
            return new SelectionInputHandler();
    }
}

bool InputHandlerFactory::isCorrectHandler(InputHandler *handler, InputMode type) {
    if (handler == nullptr) {
        return false;
    }

    if (type == InputMode::SELECTION && dynamic_cast<SelectionInputHandler *>(handler)) {
        return true;
    }

    if (type == InputMode::TEXT_FILTER && dynamic_cast<TextFilteringInputHandler *>(handler)) {
        return true;
    }

    return false;
}
