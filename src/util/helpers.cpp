#include "helpers.h"
#include <unistd.h>
#include <pwd.h>
#include <iostream>

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

void drawCenteredText(WindowManager *windowManager, const std::string &text, Dimensions position) {
    auto display = windowManager->getDisplay();
    auto window = windowManager->getWindow();

    int screen_num = DefaultScreen(display);

    GC gc = XCreateGC(display, window, 0, nullptr);

    const char * fontname = "-*-fixed-*-r-*-*-20-*-*-*-*-*-*-*";

    auto font = XLoadQueryFont (display, fontname);
    if(!font) {
        font = XLoadQueryFont(display, "fixed");
    }
    XSetFont(display, gc, font->fid);

    XSetForeground(display, gc, WhitePixel(display, screen_num));
    XSetBackground(display, gc, BlackPixel(display, screen_num));

    auto textWidth = XTextWidth(font, text.c_str(), text.length());

    XDrawString(
            windowManager->getDisplay(),
            windowManager->getWindow(),
            gc,
            (int) position.x - (textWidth / 2),
            (int) position.y,
            text.c_str(),
            (int) text.length()
    );

    XFreeGC(display, gc);
}

std::string getHomeDir() {
    std::string homedir = getpwuid(getuid())->pw_dir;

    return homedir;
}