#pragma once

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/extensions/xf86vmode.h>
#include "dimensions.h"

class WindowManager {
private:
    Dimensions getScreenDimensions();

    void setWindowSettings();
    void newWindow();

    Display* display = nullptr;
    Window window;
    XVisualInfo vInfo;
    Colormap colorMap;
public:
    Window getWindow();
    Display* getDisplay();

    void getWindowDimensions(unsigned int* width, unsigned int* height);

    void destroyWindow();

    int getDepth();

    Visual* getVisual() {
        return vInfo.visual;
    }

    Colormap getColorMap() {
        return colorMap;
    }
};
