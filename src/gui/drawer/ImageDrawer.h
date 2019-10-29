#pragma once

#include <string>
#include "ShapeDrawer.h"

class ImageDrawer : public ShapeDrawer {
protected:
    Shape calcNextShape(ShapeProperties properties, Image *hotkey, bool selected, long index) override;

    Shape drawNextShape(ShapeProperties shapeProperties, Shape shape) override;

    ShapeProperties calcShapeProps(Window window) override;

    XPoint getNextShapePosition(ShapeProperties shapeProperties, Dimensions windowDimensions) override;

    void drawSelectedShapeIndicator(ShapeProperties shapeProperties, Shape shape) override;

    void clearSelectedShapeIndicator(ShapeProperties shapeProperties, Shape shape) override;

public:
    ImageDrawer(WindowManager *windowManager) : ShapeDrawer(windowManager) {};
};



