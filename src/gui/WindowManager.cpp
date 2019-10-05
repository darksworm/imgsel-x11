#include "WindowManager.h"
#include <X11/Xatom.h>
#include <X11/extensions/Xinerama.h>

typedef struct {
    unsigned long flags;
    unsigned long functions;
    unsigned long decorations;
    long inputMode;
    unsigned long status;
} Hints;

unsigned long RGBA2DWORD(int iR, int iG, int iB, int iA)
{
    return ((iA * 256 + iR) * 256 + iG) * 256 + iB;
}

void WindowManager::setWindowSettings() {
    XClassHint *class_hints = XAllocClassHint();

    class_hints->res_name = "imgsel";
    class_hints->res_class = "imgsel";

    XStoreName(display, this->window, class_hints->res_name);
    XSetClassHint(display, this->window, class_hints);
    XFree(class_hints);

    Atom wm_delete_win = XInternAtom(display, "WM_TAKE_FOCUS", 1);
    XSetWMProtocols(display, this->window, &wm_delete_win, 1);

    Hints hints;
    Atom property;

    hints.flags = 2; // changing window decorations
    hints.decorations = 0; // disabling window decorations

    property = XInternAtom(display, "_MOTIF_WM_HINTS", true);
    XChangeProperty(display, this->window, property, property, 32, PropModeReplace, (unsigned char *) &hints, 5);

    auto window_type = XInternAtom(display, "_NET_WM_WINDOW_TYPE", false);
    property = XInternAtom(display, "_NET_WM_WINDOW_TYPE_SPLASH", false);
    XChangeProperty(display, this->window, window_type, XA_ATOM, 32, PropModeReplace, reinterpret_cast<unsigned char*>(&property), 1);

    auto windowState = XInternAtom(display, "_NET_WM_STATE", false);
    Atom windowStates[] = {
        XInternAtom(display, "_NET_WM_STATE_MODAL", false),
        XInternAtom(display, "_NET_WM_STATE_ABOVE", false),
        XInternAtom(display, "_NET_WM_STATE_STAYS_ON_TOP", false)
    };

    XChangeProperty(display, this->window, windowState, XA_ATOM, 32, PropModeReplace, reinterpret_cast<unsigned char*>(&windowStates), 1);

    property = XInternAtom(display, "I3_FLOATING_WINDOW", false);
    XChangeProperty(display, this->window, property, XA_CARDINAL, 32, PropertyNewValue, reinterpret_cast<unsigned char*>(&property), 1);

    auto screenDimensions = this->getScreenDimensions();

	XSizeHints *size_hints = XAllocSizeHints();
	size_hints->min_width = (int) screenDimensions.x;
	size_hints->min_height = (int) screenDimensions.y;
	size_hints->max_width = (int) screenDimensions.x;
	size_hints->max_height = (int) screenDimensions.y;
	size_hints->width = (int) screenDimensions.x;
	size_hints->height = (int) screenDimensions.y;
	size_hints->x = 0;
	size_hints->y = 0;
	size_hints->win_gravity = CenterGravity;
	size_hints->flags = PMinSize|PMaxSize|PPosition|PSize|PWinGravity;
	XSetWMNormalHints(display, window, size_hints);

	XFree(size_hints);

    XMapWindow(display, this->window);
    XMapRaised(display, this->window);
    XSelectInput(display, this->window, StructureNotifyMask);
    XFlush(display);
}

Display* WindowManager::getDisplay() {
    if( this->display == nullptr )
    {
        // TODO: can there be multiple displays?
        this->display = XOpenDisplay(nullptr);
    }

    return this->display;
}

Dimensions WindowManager::getScreenDimensions() {
    Display* display = this->getDisplay();

    auto screenCount = ScreenCount(display);
    if(screenCount > 1) {
        // TODO: figure out which screen is the main one
    }

    if(XineramaIsActive(display)) {
        int num;
        auto screenInfo = XineramaQueryScreens(display, &num);

        return Dimensions(screenInfo->width, screenInfo->height);
    }

    auto screen = DefaultScreenOfDisplay(display);
    return Dimensions(screen->width, screen->height);
}

void WindowManager::newWindow() {
    // TODO: handle multiple screens?
    auto screenDimensions = this->getScreenDimensions();

    int height = (int)screenDimensions.x;
    int width = (int)screenDimensions.y;

    XMatchVisualInfo(display, DefaultScreen(display), getDepth(), TrueColor, &vInfo);

    colorMap = XCreateColormap(display, DefaultRootWindow(display), vInfo.visual, AllocNone);

    XSetWindowAttributes attr;
    attr.colormap = colorMap;
    attr.border_pixel = 0;
    // TODO : this should be 0 to be transparent
    attr.background_pixel = RGBA2DWORD(0, 0, 0, 200);

    this->window = XCreateWindow(display, DefaultRootWindow(display), 0, 0, (unsigned) width, (unsigned) height, 0,
                               vInfo.depth, InputOutput,
                               vInfo.visual, CWColormap | CWBorderPixel | CWBackPixel | FocusChangeMask, &attr);

    this->setWindowSettings();
}


Window WindowManager::getWindow() {
    if( !this->window )
    {
       this->newWindow();
    }

    return this->window;
}

void WindowManager::destroyWindow() {
    XDestroyWindow( this->getDisplay(), this->getWindow() );
    XCloseDisplay( this->getDisplay() );
}

void WindowManager::getWindowDimensions(unsigned int *width, unsigned int *height) {
    int empty;
    unsigned int u_empty;
    Window root;

    XGetGeometry(display, window, &root, &empty, &empty, width, height, &u_empty, &u_empty);
}

int WindowManager::getDepth() {
    return 32;
}
