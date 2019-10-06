#pragma once
#include <X11/X.h>

struct XEventWrapper {
    int eventType;
    KeySym keySym;
};

