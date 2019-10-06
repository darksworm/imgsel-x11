#pragma once

#include <X11/Xutil.h>
#include <linux/input-event-codes.h>
#include <cstdint>

unsigned x11_keycode_to_libinput_code(KeySym key);
