#include "../DSUSB.h"

#if defined(__APPLE__) && defined(__aarch64__)

bool DSUSB_Open() { return false; }
bool DSUSB_Close() { return false; }
bool DSUSB_ShutterOpen() { return false; }
bool DSUSB_ShutterClose() { return false; }
bool DSUSB_FocusAssert() { return false; }
bool DSUSB_FocusDeassert() { return false; }
bool DSUSB_LEDOn() { return false; }
bool DSUSB_LEDOff() { return false; }
bool DSUSB_LEDRed() { return false; }
bool DSUSB_LEDGreen() { return false; }
bool DSUSB_ShutterStatus(int *status) {
    if (status) *status = 0;
    return false;
}
bool DSUSB_FocusStatus(int *status) {
    if (status) *status = 0;
    return false;
}
bool DSUSB_Reset() { return false; }

bool DSUSB2_Open() { return false; }
bool DSUSB2_Close() { return false; }
bool DSUSB2_ShutterOpen() { return false; }
bool DSUSB2_ShutterClose() { return false; }
bool DSUSB2_FocusAssert() { return false; }
bool DSUSB2_FocusDeassert() { return false; }
bool DSUSB2_LEDOn() { return false; }
bool DSUSB2_LEDOff() { return false; }
bool DSUSB2_LEDRed() { return false; }
bool DSUSB2_LEDGreen() { return false; }
bool DSUSB2_ShutterStatus(int *status) {
    if (status) *status = 0;
    return false;
}
bool DSUSB2_FocusStatus(int *status) {
    if (status) *status = 0;
    return false;
}
bool DSUSB2_Reset() { return false; }

#endif
