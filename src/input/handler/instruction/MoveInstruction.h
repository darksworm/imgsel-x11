
#include "Instruction.h"
#include "../../../gui/ImagePickerDrawer.h"

class MoveInstruction : public Instruction {
private:
    ImagePickerMove moveDirection;
    unsigned int moveSteps = 1;
public:
    ImagePickerMove getMoveDirection() {
        return this->moveDirection;
    }

    explicit MoveInstruction(ImagePickerMove moveDirection, unsigned int moveSteps) : Instruction(InstructionType::MOVE),
                                                                    moveDirection(moveDirection), moveSteps(moveSteps) {}

    unsigned int getMoveSteps() const {
        return moveSteps;
    }
};