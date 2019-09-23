#pragma once

#include "ImageDrawer.h"

enum class ShapeType {
    IMAGE
};

class ShapeDrawerFactory {
public:
    static ShapeDrawer *getShapeDrawer(ShapeType shapeType, WindowManager *windowManager);
};

