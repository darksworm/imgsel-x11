#include <utility>

#include "ImagePickerDrawer.h"
#include "../exceptions/OutOfBounds.h"
#include "dimensions.h"
#include <memory>

ImagePickerDrawer::ImagePickerDrawer(WindowManager *windowManager, std::vector<Image> *images) {
    this->windowManager = windowManager;
    this->page = 0;
    this->selectedShape = nullptr;
    this->allImages = images;
    this->images = new std::vector<Image>(images->begin(), images->end());

    shapeDrawer = ShapeDrawerFactory::getShapeDrawer(ShapeType::IMAGE, windowManager);
    shapeProperties = shapeDrawer->calcShapeProps(windowManager->getWindow());
}

void ImagePickerDrawer::drawFrame(Image *selectedImage) {
    auto start = getPageImageStart();

    shapes.clear();
    shapeDrawer->lastShapePosition = nullptr;

    std::unique_ptr<Dimensions> windowDimensions(new Dimensions);
    windowManager->getWindowDimensions(&windowDimensions->x, &windowDimensions->y);

    unsigned int shapeCnt = shapeProperties.itemCounts.x * shapeProperties.itemCounts.y;
    int drawnShapeCnt = 0;

    auto it = start;

    for (; it != images->end(); ++it) {
        if (filter && !filter(&*it)) {
            continue;
        }

        if (selectedImage == nullptr) {
            selectedImage = &*it;
        }

        bool selected =  &*it == selectedImage;

        Shape shape{
                .selected = selected,
                .index = std::distance(images->begin(), it),
                .image = &*it
        };

        shape = shapeDrawer->drawNextShape(shapeProperties, *windowDimensions, shape);

        shapes.emplace(shape.index, shape);

        if (selected) {
            this->selectedShape = &(--this->shapes.end())->second;
        }

        if (++drawnShapeCnt >= shapeCnt) {
            break;
        }
    }

    bool hasNextPage = false;

    if (!filter) {
        hasNextPage = it + 1 < images->end();
    } else {
        if (++it >= images->end()) {
            hasNextPage = false;
        } else {
            for (; it != images->end(); ++it) {
                if (filter && !filter(&*it)) {
                    continue;
                } else {
                    hasNextPage = true;
                    break;
                }
            }
        }
    }

    int circleTypes[] = {0, 0, 0};

    if (hasNextPage) {
        circleTypes[page == 0 ? 0 : 1] = 1;
    } else {
        circleTypes[2] = 1;
    }

    GC gc = XCreateGC(windowManager->getDisplay(), windowManager->getWindow(), 0, nullptr);

    XSetForeground(windowManager->getDisplay(), gc,
                   WhitePixel(windowManager->getDisplay(), DefaultScreen(windowManager->getDisplay())));
    XSetBackground(windowManager->getDisplay(), gc, DefaultScreen(windowManager->getDisplay()));
    XSetFillStyle(windowManager->getDisplay(), gc, FillSolid);
    XSetLineAttributes(windowManager->getDisplay(), gc, 2, LineSolid, CapRound, JoinRound);

    int i = 0;
    for (const int &circleType : circleTypes) {
        auto dia = 15;
        auto spacing = 10;
        auto xPos = windowDimensions->x - 35;
        auto baseYPos = windowDimensions->y / 2 - (dia * 1.5) - spacing;
        auto yPos = baseYPos + (i * (dia + spacing));

        XDrawArc(windowManager->getDisplay(), windowManager->getWindow(), gc, xPos, yPos, dia, dia, 0, 360 * 64);
        if (circleType == 1) {
            XFillArc(windowManager->getDisplay(), windowManager->getWindow(), gc, xPos, yPos, dia, dia, 0, 360 * 64);
        }
        i++;
    }
}

std::vector<Image>::iterator ImagePickerDrawer::getPageImageStart() {
    int hotkeysPerPage = shapeProperties.itemCounts.y * shapeProperties.itemCounts.x;

    preloadToIndex(hotkeysPerPage - 1);

    if (page > 0 && images->size() < hotkeysPerPage) {
        throw OutOfBounds();
    }

    int offset = hotkeysPerPage * page;

    return images->begin() + offset;
}

void ImagePickerDrawer::preloadToIndex(unsigned int targetIndex) {
    if(filter && images->size() < (long)targetIndex + 1) {
        std::cout << "Loading page...\n";
        unsigned int offset = 0;

        if(lastPreloadedImageIndex) {
            if(allImages->begin() + lastPreloadedImageIndex + 1 == allImages->end()) {
                return;
            } else {
                offset = lastPreloadedImageIndex + 1;
            }
        }

        unsigned int hotkeysPerPage = shapeProperties.itemCounts.y * shapeProperties.itemCounts.x;
        unsigned int targetImageCount = ((targetIndex / hotkeysPerPage) + 1) * hotkeysPerPage;

        for (auto it = allImages->begin() + offset; it != allImages->end(); ++it) {
            if(filter(&*it)) {
                images->push_back(*it);
                lastPreloadedImageIndex = std::distance(allImages->begin(), it);

                if(images->size() >= targetImageCount) {
                    break;
                }
            }
        }
    }
}

int ImagePickerDrawer::getImagePage(long index) {
    return (int) (index / (this->shapeProperties.itemCounts.x * this->shapeProperties.itemCounts.y));
}

void ImagePickerDrawer::goToImage(long hotkeyIdx) {
    Image *image = &*(images->begin() + hotkeyIdx);

    page = getImagePage(hotkeyIdx);
    drawFrame(image);
}

bool ImagePickerDrawer::move(ImagePickerMove move, unsigned int steps) {
    bool canMove = false;
    long newSelectedShapeIdx = 0;

    char *debug;

    switch (move) {
        case ImagePickerMove::LEFT:
            canMove = selectedShape->index >= steps;
            newSelectedShapeIdx = selectedShape->index - steps;
            debug = "LEFT";
            break;
        case ImagePickerMove::RIGHT:
            preloadToIndex(selectedShape->index + steps);
            canMove = selectedShape->index + steps < images->size();
            newSelectedShapeIdx = selectedShape->index + steps;
            debug = "RIGHT";
            break;
        case ImagePickerMove::UP:
            canMove = selectedShape->index - (steps * shapeProperties.itemCounts.x) >= 0;
            newSelectedShapeIdx = selectedShape->index - (steps * shapeProperties.itemCounts.x);
            debug = "UP";
            break;
        case ImagePickerMove::DOWN:
            preloadToIndex(selectedShape->index + (steps * shapeProperties.itemCounts.x));
            canMove = selectedShape->index + (steps * shapeProperties.itemCounts.x) < images->size();
            newSelectedShapeIdx = selectedShape->index + (steps * shapeProperties.itemCounts.x);
            debug = "DOWN";
            break;
        case ImagePickerMove::END:
            preloadToIndex(INT_MAX);
            canMove = selectedShape->index != images->size() - 1;
            newSelectedShapeIdx = images->size() - 1;
            debug = "END";
            break;
        case ImagePickerMove::HOME:
            canMove = true;
            newSelectedShapeIdx = 0;
            debug = "HOME";
            break;
        case ImagePickerMove::LINE:
            preloadToIndex(steps > 0 && shapeProperties.itemCounts.x * steps);
            canMove = steps > 0 && shapeProperties.itemCounts.x * steps < images->size();
            newSelectedShapeIdx = shapeProperties.itemCounts.x * (steps - 1);
            debug = "LINE";
    }

    printf("type: %s, canmove: %d oldIdx: %d newIdx: %d \n", debug, canMove, selectedShape->index,
           (int) newSelectedShapeIdx);

    if (canMove) {
        goToImage(newSelectedShapeIdx);
    }

    return canMove;
}

Image *ImagePickerDrawer::getSelectedImage() {
    if (selectedShape != nullptr) {
        return selectedShape->image;
    } else {
        return nullptr;
    }
}

void ImagePickerDrawer::setFilter(std::function<bool(Image *)> filter, std::string filterString) {
    this->filter = std::move(filter);
    this->filterString = filterString;

    lastPreloadedImageIndex = 0;
    images->clear();

    if(filterString.empty()) {
        delete(this->images);
        this->images = new std::vector<Image>(allImages->begin(), allImages->end());
    }
}
