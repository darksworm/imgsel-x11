#pragma once

void handleEvents(WindowManager *windowManager, ThreadSafeQueue<XEventWrapper> *eventQueue,
                  std::vector<Image> images, bool *shouldQuit);
