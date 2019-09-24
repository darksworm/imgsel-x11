#pragma once

#include "../InputMode.h"
#include "Instruction.h"

class CopyInstruction : public Instruction {
private:
public:
    explicit CopyInstruction()
            : Instruction(InstructionType::COPY) {};
};
