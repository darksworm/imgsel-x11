#include <thread>
#include "../gui/WindowManager.h"
#include "../util/ThreadSafeQueue.h"
#include "XEventWrapper.h"
#include "XInputHandler.h"

void handleXInput(WindowManager *windowManager, ThreadSafeQueue<XEventWrapper> *eventQueue, bool *shouldQuit) {
    auto display = windowManager->getDisplay();
    auto window = windowManager->getWindow();

    XSelectInput(display, window, ExposureMask | KeyPressMask | KeyReleaseMask | ButtonPressMask | StructureNotifyMask);
    XEvent event;

    bool keepRunning = true;

    while (keepRunning) {
        while (XPending(display)) {
            XNextEvent(display, &event);

            switch (event.type) {
                case DestroyNotify:
                case UnmapNotify:
                    eventQueue->push(XEventWrapper{
                            .eventType = -1
                    });
                    keepRunning = false;
                    break;

                case KeyRelease:
                case KeyPress: {
                    eventQueue->push(XEventWrapper{
                            .eventType = event.type,
                            .keySym = XLookupKeysym(&event.xkey, 0)
                    });
                    break;
                }

                default:
                    eventQueue->push(XEventWrapper{
                            .eventType = event.type
                    });
                    break;
            }
        }
        std::this_thread::sleep_for(std::chrono::nanoseconds(16600000));

        if (*shouldQuit) {
            keepRunning = false;
        }
    }
}


