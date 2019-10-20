#include <Imlib2.h>
#include "ImageDrawer.h"
#include "../../exceptions/ImageNotLoadable.h"
#include "../../config/ConfigManager.h"


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
        throw ImageNotLoadable();
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

    XPoint imagePos = XPoint();
    imagePos.x = pos->x + shapeProperties.dimensions.x / 2 - width / 2;
    imagePos.y = pos->y + shapeProperties.dimensions.y / 2 - height / 2;

    imlib_render_image_on_drawable(imagePos.x, imagePos.y);

    shape.position.x = pos->x;
    shape.position.y = pos->y;
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
    auto config = ConfigManager::getOrLoadConfig();

    // TODO: calculate missing props dynamically
    ShapeProperties shapeProperties{
            .dimensions = Dimensions(300, 150),
            .margins = Dimensions(20, 20),
            .itemCounts = Dimensions(config.getCols() > 0 ? config.getCols() : 4, config.getRows() > 0 ? config.getRows() : 4),
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

    unsigned int xMargin, yMargin;

    unsigned int oneRowWidth = shapeProperties.dimensions.x * shapeProperties.itemCounts.x +
                               (shapeProperties.margins.x * shapeProperties.itemCounts.x - 1);
    unsigned int oneColumnHeight = shapeProperties.dimensions.y * shapeProperties.itemCounts.y +
                                   (shapeProperties.margins.y * shapeProperties.itemCounts.y - 1);

    xMargin = (windowDimensions.x - oneRowWidth) / 2;
    yMargin = (windowDimensions.y - oneColumnHeight) / 2;

    if (!lastShapePosition) {
        newShapePosition = new XPoint{
                .x = (short) xMargin,
                .y = (short) yMargin
        };
    } else {
        XPoint offset;

        if (lastShapePosition->x >= shapeProperties.dimensions.x * (shapeProperties.itemCounts.x - 1) + xMargin) {
            // move to next line
            offset.y = (short) (shapeProperties.dimensions.y + shapeProperties.margins.y + lastShapePosition->y);
            offset.x = (short) xMargin;
        } else {
            offset.x = (short) (lastShapePosition->x + shapeProperties.dimensions.x + shapeProperties.margins.x);
            offset.y = (short) (lastShapePosition->y);
        }

        newShapePosition = new XPoint{
                .x = (short) (offset.x),
                .y = (short) (offset.y)
        };
    }

    return newShapePosition;
}

void ImageDrawer::drawSelectedShapeIndicator(ShapeProperties shapeProperties, Shape shape) {
    if (shape.selected) {
        auto pos = shape.position;
        XDrawRectangle(windowManager->getDisplay(), windowManager->getWindow(), selectedShapeGC, pos.x, pos.y,
                       shapeProperties.dimensions.x, shapeProperties.dimensions.y);
    }
}

void ImageDrawer::clearSelectedShapeIndicator(ShapeProperties shapeProperties, Shape shape) {
    auto pos = shape.position;

    XClearArea(
            windowManager->getDisplay(),
            windowManager->getWindow(),
            pos.x - selectedShapeLineWidth,
            pos.y - selectedShapeLineWidth,
            shapeProperties.dimensions.x + (2 * selectedShapeLineWidth),
            selectedShapeLineWidth * 2,
            false
    );

    XClearArea(
            windowManager->getDisplay(),
            windowManager->getWindow(),
            pos.x - selectedShapeLineWidth,
            pos.y - selectedShapeLineWidth + shapeProperties.dimensions.y,
            shapeProperties.dimensions.x + (2 * selectedShapeLineWidth),
            selectedShapeLineWidth * 2,
            false
    );

    XClearArea(
            windowManager->getDisplay(),
            windowManager->getWindow(),
            pos.x - selectedShapeLineWidth,
            pos.y - selectedShapeLineWidth,
            (2 * selectedShapeLineWidth),
            shapeProperties.dimensions.y + selectedShapeLineWidth * 2,
            false
    );

    XClearArea(
            windowManager->getDisplay(),
            windowManager->getWindow(),
            pos.x - selectedShapeLineWidth + shapeProperties.dimensions.x,
            pos.y - selectedShapeLineWidth,
            (2 * selectedShapeLineWidth),
            shapeProperties.dimensions.y + selectedShapeLineWidth * 2,
            false
    );
}
