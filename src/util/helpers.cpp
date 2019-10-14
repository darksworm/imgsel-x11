#include "helpers.h"

void drawText(WindowManager *windowManager, const std::string &text, Dimensions position) {
    XClearArea(
            windowManager->getDisplay(),
            windowManager->getWindow(),
            position.x - 10,
            position.y - 10,
            320,
            60,
            false
    );

    auto display = windowManager->getDisplay();
    auto window = windowManager->getWindow();

    // TODO: these should be config options
    int screen_num = DefaultScreen(display);

    GC gc = XCreateGC(display, window, 0, nullptr);

    XSetForeground(display, gc, WhitePixel(display, screen_num));
    XSetBackground(display, gc, BlackPixel(display, screen_num));

    XDrawString(
            windowManager->getDisplay(),
            windowManager->getWindow(),
            gc,
            (int) position.x,
            (int) position.y,
            text.c_str(),
            (int) text.length()
    );

    XFreeGC(display, gc);
}

