#pragma once

#include <string>
#include "ShapeDrawer.h"

class ImageDrawer : public ShapeDrawer {
protected:
    Shape calcNextShape(ShapeProperties properties, Image *hotkey, bool selected, long index) override;

    Shape drawNextShape(ShapeProperties shapeProperties, Dimensions windowDimensions, Shape shape) override;

    ShapeProperties calcShapeProps(Window window) override;

    XPoint *getNextShapePosition(ShapeProperties shapeProperties, Dimensions windowDimensions) override;

public:
    ImageDrawer(WindowManager *windowManager) : ShapeDrawer(windowManager) {};
};



