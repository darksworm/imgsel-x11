#include "ImageDrawer.h"


Shape ImageDrawer::calcNextShape(ShapeProperties properties, Image *hotkey, bool selected, long index) {
    return Shape();
}

Shape ImageDrawer::drawNextShape(ShapeProperties shapeProperties, Dimensions windowDimensions, Shape shape) {
    return Shape();
}

ShapeProperties ImageDrawer::calcShapeProps(Window window) {
    return ShapeProperties();
}

XPoint *ImageDrawer::getNextShapePosition(ShapeProperties shapeProperties, Dimensions windowDimensions) {
    return nullptr;
}
