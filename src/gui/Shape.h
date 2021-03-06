#pragma once

#include <X11/Xutil.h>
#include "../image/image.h"
#include "dimensions.h"

struct ShapeProperties {
    Dimensions dimensions;
    Dimensions margins;
    Dimensions itemCounts;

    XRectangle nameRect;
    XPoint position;
};

struct Shape {
    XPoint position;

    bool selected;
    long index;

    Image *image;
};

