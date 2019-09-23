
#include "ShapeDrawerFactory.h"
#include "ImageDrawer.h"

ShapeDrawer *ShapeDrawerFactory::getShapeDrawer(ShapeType shapeType, WindowManager *windowManager) {
    switch (shapeType) {
        case ShapeType::IMAGE:
            return new ImageDrawer(windowManager);
        default:
            return nullptr;
    }
}
