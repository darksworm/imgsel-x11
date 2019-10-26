#pragma once


#include "InputInstructionType.h"

class InputInstruction {
public:
    explicit InputInstruction(const InputInstructionType &type) : type(type) {};

    InputInstructionType getType() {
        return type;
    };

    virtual ~InputInstruction() {};

private:
    InputInstructionType type;
};



