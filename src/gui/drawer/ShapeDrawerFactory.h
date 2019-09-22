#pragma once

#include "RectangleShapeDrawer.h"

enum class ShapeType {
    RECTANGLE,
    IMAGE
};

class ShapeDrawerFactory {
public:
    static ShapeDrawer *getShapeDrawer(ShapeType shapeType, WindowManager *windowManager);
};

