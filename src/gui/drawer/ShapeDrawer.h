#pragma once

#include "../Shape.h"
#include "../WindowManager.h"
#include <climits>
#include <numeric>

class ShapeDrawer {
    friend class ImagePickerDrawer;

private:
    GC createShapeGC(WindowManager *windowManager) {
        auto display = windowManager->getDisplay();
        auto window = windowManager->getWindow();

        // TODO: these should be config options
        int screen_num = DefaultScreen(display);
        unsigned int line_width = 2;
        int line_style = LineSolid;
        int cap_style = CapButt;
        int join_style = JoinBevel;

        GC gc = XCreateGC(display, window, 0, nullptr);

        XSetForeground(display, gc, WhitePixel(display, screen_num));
        XSetBackground(display, gc, BlackPixel(display, screen_num));

        XSetLineAttributes(display, gc,
                           line_width, line_style, cap_style, join_style);
        XSetFillStyle(display, gc, FillSolid);

        return gc;
    }

    GC createSelectedShapeGC(WindowManager *windowManager) {
        auto display = windowManager->getDisplay();
        auto window = windowManager->getWindow();

        // TODO: these should be config options
        int screen_num = DefaultScreen(display);
        int line_style = LineSolid;
        int cap_style = CapButt;
        int join_style = JoinBevel;

        GC gc = XCreateGC(display, window, 0, nullptr);

        XColor color;
        color.red = USHRT_MAX;
        color.green = 0;
        color.blue = 0;

        color.flags = DoRed | DoGreen | DoBlue;
        XAllocColor(display, DefaultColormap(display, screen_num), &color);

        XSetForeground(display, gc, color.pixel);
        XSetBackground(display, gc, BlackPixel(display, screen_num));

        XSetLineAttributes(display, gc,
                           selectedShapeLineWidth, line_style, cap_style, join_style);
        XSetFillStyle(display, gc, FillSolid);

        return gc;
    }

    GC createTextGC(WindowManager *windowManager) {
        auto display = windowManager->getDisplay();
        auto window = windowManager->getWindow();

        // TODO: these should be config options
        int screen_num = DefaultScreen(display);

        GC gc = XCreateGC(display, window, 0, nullptr);

        XSetForeground(display, gc, WhitePixel(display, screen_num));
        XSetBackground(display, gc, BlackPixel(display, screen_num));

        return gc;
    }

protected:
    GC shapeGC;
    GC selectedShapeGC;
    GC textGC;
    unsigned int selectedShapeLineWidth = 2;

    WindowManager *windowManager;
    XPoint *lastShapePosition = nullptr;

    virtual Shape calcNextShape(ShapeProperties properties, Image *hotkey, bool selected, long index) = 0;

    virtual Shape drawNextShape(ShapeProperties shapeProperties, Dimensions windowDimensions, Shape shape) = 0;

    virtual ShapeProperties calcShapeProps(Window window) = 0;

    virtual XPoint *getNextShapePosition(ShapeProperties shapeProperties, Dimensions windowDimensions) = 0;

    virtual void drawSelectedShapeIndicator(ShapeProperties shapeProperties, Shape shape) = 0;

    virtual void clearSelectedShapeIndicator(ShapeProperties shapeProperties, Shape shape) = 0;

public:
    ShapeDrawer(WindowManager *windowManager) {
        shapeGC = createShapeGC(windowManager);
        selectedShapeGC = createSelectedShapeGC(windowManager);
        textGC = createTextGC(windowManager);

        this->windowManager = windowManager;
    }

    ~ShapeDrawer() {
        delete this->lastShapePosition;

        XFreeGC(this->windowManager->getDisplay(), textGC);
        XFreeGC(this->windowManager->getDisplay(), selectedShapeGC);
        XFreeGC(this->windowManager->getDisplay(), shapeGC);
    }
};
