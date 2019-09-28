#include <Imlib2.h>
#include "ImageDrawer.h"


Shape ImageDrawer::calcNextShape(ShapeProperties properties, Image *hotkey, bool selected, long index) {
    return Shape();
}

Shape ImageDrawer::drawNextShape(ShapeProperties shapeProperties, Dimensions windowDimensions, Shape shape) {
    Imlib_Image img;
    Pixmap pix;
    int width, height;

    img = imlib_load_image(shape.image->getPath().c_str());
    if (!img) {
        fprintf(stderr, "%s:Unable to load image\n", shape.image->getPath().c_str());
        throw 0;
    }

    imlib_context_set_image(img);
    width = imlib_image_get_width();
    height = imlib_image_get_height();

    pix = XCreatePixmap(windowManager->getDisplay(), windowManager->getWindow(), width, height,
                        windowManager->getDepth());


    imlib_context_set_display(windowManager->getDisplay());
    imlib_context_set_visual(windowManager->getVisual());
    imlib_context_set_colormap(windowManager->getColorMap());
    imlib_context_set_drawable(windowManager->getWindow());

    XPoint *pos = getNextShapePosition(shapeProperties, windowDimensions);

    if (shape.selected) {
        XDrawRectangle(windowManager->getDisplay(), windowManager->getWindow(), selectedShapeGC, pos->x, pos->y,
                       shapeProperties.dimensions.x, shapeProperties.dimensions.y);
    }

    XPoint imagePos = XPoint();
    imagePos.x = pos->x + shapeProperties.dimensions.x / 2 - width / 2;
    imagePos.y = pos->y + shapeProperties.dimensions.y / 2 - height / 2;

    imlib_render_image_on_drawable(imagePos.x, imagePos.y);

    shape.position = *pos;
    lastShapePosition = pos;

    imlib_free_image();
    XFreePixmap(windowManager->getDisplay(), pix);

    XDrawString(
            windowManager->getDisplay(),
            windowManager->getWindow(),
            textGC,
            shape.position.x + shapeProperties.botTextRect.x,
            shape.position.y + shapeProperties.botTextRect.y,
            shape.image->getFilename().c_str(),
            (int) shape.image->getFilename().length()
    );

    return shape;
}

ShapeProperties ImageDrawer::calcShapeProps(Window window) {
    // TODO: calculate dynamically
    ShapeProperties shapeProperties{
            .dimensions = Dimensions(300, 150),
            .margins = Dimensions(20, 20),
            .itemCounts = Dimensions(4, 4),
            .topTextRect = XRectangle{
                    .x = 10,
                    .y = 20,
                    .width = (unsigned short) (shapeProperties.dimensions.x - 10),
                    .height = 40
            },
            .midTextRect = XRectangle{
                    .x = shapeProperties.topTextRect.x,
                    .y = (short) (shapeProperties.dimensions.y - 80),
                    .width = shapeProperties.topTextRect.width,
                    .height = shapeProperties.topTextRect.height
            },
            .botTextRect = XRectangle{
                    .x = shapeProperties.topTextRect.x,
                    .y = (short) (shapeProperties.dimensions.y - 40),
                    .width = shapeProperties.topTextRect.width,
                    .height = shapeProperties.topTextRect.height
            }
    };

    return shapeProperties;
}

XPoint *ImageDrawer::getNextShapePosition(ShapeProperties shapeProperties, Dimensions windowDimensions) {
    XPoint *lastShapePosition = this->lastShapePosition;
    XPoint *newShapePosition;

    unsigned int xCenterMargin, yCenterMargin;

    unsigned int line_width = shapeProperties.dimensions.x * shapeProperties.itemCounts.x +
                              (shapeProperties.margins.x * shapeProperties.itemCounts.x - 1);
    unsigned int line_height = shapeProperties.dimensions.y * shapeProperties.itemCounts.y +
                               (shapeProperties.margins.y * shapeProperties.itemCounts.y - 1);

    xCenterMargin = (windowDimensions.x - line_width) / 2;
    yCenterMargin = (windowDimensions.y - line_height) / 2;

    if (!lastShapePosition) {
        newShapePosition = new XPoint{
                .x = (short) xCenterMargin,
                .y = (short) yCenterMargin
        };
    } else {
        XPoint offset;

        if (lastShapePosition->x >= shapeProperties.dimensions.x * (shapeProperties.itemCounts.x - 1) + xCenterMargin) {
            // move to next line
            offset.y = (short) (shapeProperties.dimensions.y + shapeProperties.margins.y + lastShapePosition->y +
                                shapeProperties.margins.y);
            offset.x = (short) xCenterMargin;
        } else {
            offset.x = (short) (lastShapePosition->x + shapeProperties.dimensions.x + shapeProperties.margins.x);
            offset.y = (short) (lastShapePosition->y);
        }

        // TODO: this is temporary positioning
        newShapePosition = new XPoint{
                .x = (short) (offset.x),
                .y = (short) (offset.y)
        };
    }

    return newShapePosition;
}
