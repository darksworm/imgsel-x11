#include <Imlib2.h>
#include <png.h>
#include "ImageDrawer.h"
#include "../../exceptions/ImageNotLoadable.h"
#include "../../config/ConfigManager.h"


Shape ImageDrawer::calcNextShape(ShapeProperties properties, Image *hotkey, bool selected, long index) {
    return Shape();
}


GC create_gc(Display *display, Window win, int reverse_video) {
    GC gc;                /* handle of newly created GC.  */
    unsigned long valuemask = 0;      /* which values in 'values' to  */

/* check when creating the GC.  */
    XGCValues values;         /* initial values for the GC.   */
    unsigned int line_width = 2;      /* line width for the GC.       */
    int line_style = LineSolid;       /* style for lines drawing and  */
    int cap_style = CapButt;      /* style of the line's edje and */
    int join_style = JoinBevel;       /*  joined lines.       */
    int screen_num = DefaultScreen(display);
    gc = XCreateGC(display, win, valuemask, &values);
/* allocate foreground and background colors for this GC. */
    if (reverse_video) {
        XSetForeground(display, gc, WhitePixel(display, screen_num));
        XSetBackground(display, gc, BlackPixel(display, screen_num));
    } else {
        XSetForeground(display, gc, BlackPixel(display, screen_num));
        XSetBackground(display, gc, WhitePixel(display, screen_num));
    }
    XSetLineAttributes(display, gc, line_width, line_style, cap_style, join_style);
    XSetFillStyle(display, gc, FillSolid);

    return gc;

}

static void TeardownPng(png_structp png, png_infop info) {

    if (png) {
        png_infop *realInfo = (info ? &info : NULL);
        png_destroy_read_struct(&png, realInfo, NULL);
    }
}

void LoadPng(FILE *file, unsigned char **data, char **clipData, unsigned int *width, unsigned int *height,
             unsigned int *rowbytes) {
    size_t size = 0, clipSize = 0;
    png_structp png = NULL;
    png_infop info = NULL;
    unsigned char **rowPointers = NULL;
    int depth = 0,
            colortype = 0,
            interlace = 0,
            compression = 0,
            filter = 0;
    unsigned clipRowbytes = 0;
    png = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    info = png_create_info_struct(png);
    png_init_io(png, file);
    png_read_info(png, info);
    png_get_IHDR(png, info, (png_uint_32 *) width, (png_uint_32 *) height, &depth, &colortype, &interlace, &compression,
                 &filter);
    *rowbytes = png_get_rowbytes(png, info);
    if (colortype == PNG_COLOR_TYPE_RGB) {
        // X hates 24bit images - pad to RGBA
        png_set_filler(png, 0xff, PNG_FILLER_AFTER);
        *rowbytes = (*rowbytes * 4) / 3;
    }
    png_set_bgr(png);
    *width = png_get_image_width(png, info);
    *height = png_get_image_height(png, info);
    size = *height * *rowbytes;
    clipRowbytes = *rowbytes / 32;
    if (*rowbytes % 32)
        ++clipRowbytes;
    clipSize = clipRowbytes * *height;
    // This gets freed by XDestroyImage
    *data = (unsigned char *) malloc(sizeof(png_byte) * size);
    rowPointers = (unsigned char **) malloc(*height * sizeof(unsigned char *));
    png_bytep cursor = *data;
    int i = 0, x = 0, y = 0;
    for (i = 0; i < *height; ++i, cursor += *rowbytes)
        rowPointers[i] = cursor;
    png_read_image(png, rowPointers);
    *clipData = (char *) calloc(clipSize, sizeof(unsigned char));
    if (colortype == PNG_COLOR_TYPE_RGB) {
        memset(*clipData, 0xff, clipSize);
    } else {
        // Set up bitmask for clipping fully transparent areas
        for (y = 0; y < *height; ++y, cursor += *rowbytes) {
            for (x = 0; x < *rowbytes; x += 4) {
                // Set bit in mask when alpha channel is nonzero
                if (rowPointers[y][x + 3])
                    (*clipData)[(y * clipRowbytes) + (x / 32)] |= (1 << ((x / 4) % 8));
            }
        }
    }
    TeardownPng(png, info);
    free(rowPointers);
}


Shape ImageDrawer::drawNextShape(ShapeProperties shapeProperties, Shape shape) {
    auto config = ConfigManager::getOrLoadConfig();
    Imlib_Image img;
    Pixmap pix;
    int width, height;

    unsigned width_ = 0, height_ = 0;
    unsigned char *data = NULL;
    char *clip = NULL;
    unsigned rowbytes = 0;

    FILE *file = fopen((char *) shape.image->getPath().c_str(), "r");
    std::cout << "loading file " << shape.image->getPath().c_str() << "\n";

    try {
        LoadPng(file, &data, &clip, &width_, &height_, &rowbytes);
    } catch (...) {
        throw ImageNotLoadable();
    }

    XImage *ximage = XCreateImage(windowManager->getDisplay(), DefaultVisual(windowManager->getDisplay(), DefaultScreen(
            windowManager->getDisplay())),
                                  32, ZPixmap, 0, (char *) data, width_,
                                  height_, 8, rowbytes);

    if (!ximage) {
        std::cout << ("Unable to load image", shape.image->getPath().c_str()) << "\n";
        throw ImageNotLoadable();
    }

//    img = imlib_load_image(shape.image->getPath().c_str());
//
//    if (!img) {
//        spdlog::error("Unable to load image {}", shape.image->getPath().c_str());
//        throw ImageNotLoadable();
//    }
//
//    imlib_context_set_image(img);
//    width = imlib_image_get_width();
//    height = imlib_image_get_height();
//
//    if (config.getMaxImageHeight() + config.getMaxImageWidth() > 0) {
//        if (config.getMaxImageWidth() > 0 && width > config.getMaxImageWidth()) {
//            auto scale = (double)config.getMaxImageWidth() / width;
//            int new_height = height * scale;
//            img = imlib_create_cropped_scaled_image(0, 0, width, height, config.getMaxImageWidth(), new_height);
//            imlib_free_image();
//            imlib_context_set_image(img);
//
//            width = config.getMaxImageWidth();
//            height = new_height;
//        }
//
//        if (config.getMaxImageHeight() > 0 && height > config.getMaxImageHeight()) {
//            auto scale = (double)config.getMaxImageHeight() / height;
//            int new_width = width * scale;
//            img = imlib_create_cropped_scaled_image(0, 0, width, height, new_width, config.getMaxImageHeight());
//            imlib_free_image();
//            imlib_context_set_image(img);
//
//            width = new_width;
//            height = config.getMaxImageHeight();
//        }
//    }
//
//    pix = XCreatePixmap(windowManager->getDisplay(), windowManager->getWindow(), width, height,
//                        windowManager->getDepth());
//
//    imlib_context_set_display(windowManager->getDisplay());
//    imlib_context_set_visual(windowManager->getVisual());
//    imlib_context_set_colormap(windowManager->getColorMap());
//    imlib_context_set_drawable(windowManager->getWindow());

    XPoint imagePos = XPoint();
    imagePos.x = shapeProperties.position.x + shapeProperties.dimensions.x / 2 - width_ / 2;
    imagePos.y = shapeProperties.position.y + shapeProperties.dimensions.y / 2 - height_ / 2;

//    imlib_render_image_on_drawable(imagePos.x, imagePos.y);

    auto gc = create_gc(windowManager->getDisplay(), windowManager->getWindow(), 0);

    XPutImage(windowManager->getDisplay(), windowManager->getWindow(), gc, ximage, 0, 0, imagePos.x, imagePos.y, width_,
              height_);

    lastShapePosition = shapeProperties.position;

//    imlib_free_image();
//    XFreePixmap(windowManager->getDisplay(), pix);

    auto font = XLoadQueryFont(windowManager->getDisplay(), "fixed");
    XSetFont(windowManager->getDisplay(), textGC, font->fid);

    auto displayName = shape.image->getFilenameWithoutExtension();
    unsigned int textWidth = 0;
    auto maxTextWidth = shapeProperties.dimensions.x - 20;

    do {
        textWidth = XTextWidth(font, displayName.c_str(), displayName.length());

        if (textWidth >= maxTextWidth) {
            displayName = displayName.substr(0, displayName.length() - 1);
        }
    } while (textWidth >= maxTextWidth);

    XDrawString(
            windowManager->getDisplay(),
            windowManager->getWindow(),
            textGC,
            lastShapePosition.value().x + shapeProperties.dimensions.x / 2 - textWidth / 2,
            lastShapePosition.value().y + shapeProperties.nameRect.y,
            displayName.c_str(),
            (int) displayName.length()
    );

    return shape;
}

ShapeProperties ImageDrawer::calcShapeProps(Window window) {
    auto config = ConfigManager::getOrLoadConfig();

    ShapeProperties shapeProperties{
            .dimensions = Dimensions(config.getMaxImageWidth() + config.getXPadding() * 2,
                                     config.getMaxImageHeight() + config.getYPadding() * 2),
            .margins = Dimensions(config.getXMargin(), config.getYMargin()),
            .itemCounts = Dimensions(config.getCols() > 0 ? config.getCols() : 4,
                                     config.getRows() > 0 ? config.getRows() : 4),
            .nameRect = XRectangle{
                    .x = 0,
                    .y = static_cast<short>(config.getMaxImageHeight() + config.getYPadding() * 1.5),
                    .width = 0,
                    .height = 0
            },
            .position = XPoint()
    };

    return shapeProperties;
}

XPoint ImageDrawer::getNextShapePosition(ShapeProperties shapeProperties, Dimensions windowDimensions) {
    auto lastShapePosition = this->lastShapePosition;
    XPoint newShapePosition;

    unsigned int xMargin, yMargin;

    unsigned int oneRowWidth = shapeProperties.dimensions.x * shapeProperties.itemCounts.x +
                               (shapeProperties.margins.x * shapeProperties.itemCounts.x - 1);
    unsigned int oneColumnHeight = shapeProperties.dimensions.y * shapeProperties.itemCounts.y +
                                   (shapeProperties.margins.y * shapeProperties.itemCounts.y - 1);

    xMargin = (windowDimensions.x - oneRowWidth) / 2;
    yMargin = (windowDimensions.y - oneColumnHeight) / 2;

    if (!lastShapePosition.has_value()) {
        newShapePosition = XPoint{
                .x = (short) xMargin,
                .y = (short) yMargin
        };
    } else {
        XPoint offset;

        if (lastShapePosition->x + shapeProperties.margins.x + shapeProperties.dimensions.x > oneRowWidth + xMargin) {
            // move to next line
            offset.y = (short) (shapeProperties.dimensions.y + shapeProperties.margins.y + lastShapePosition->y);
            offset.x = (short) xMargin;
        } else {
            offset.x = (short) (lastShapePosition->x + shapeProperties.dimensions.x + shapeProperties.margins.x);
            offset.y = (short) (lastShapePosition->y);
        }

        newShapePosition = XPoint{
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


