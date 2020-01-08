#include "testcases.hpp"

#include <thread>
#include <chrono>

#include "../toggleswitch.hpp"
#include "../flags.hpp"

void sleep(long long ms) {
    using namespace std::this_thread;
    using namespace std::chrono;
    sleep_for(milliseconds(ms));
}

int buttonSyncTest() {
    // Create a Hardpoints toggle switch and turn it on
    Toggleswitch ts(BUTTON_RELEASED, 0, FLAG_HARDPOINTS);
    ts.currState = BUTTON_PRESSED;
    ts.update(0, true);

    // Initial button press
    if (!buttonPressed[0]) {
        return 1;
    }

    sleep(BUTTON_HOLD_TIME * 2);
    ts.update(0, true);

    // Button release
    if (buttonPressed[0]) {
        return 1;
    }

    sleep(BUTTON_SYNC_TIME);
    ts.update(0, true);

    // Wait for the sync time and tell the switch the flag has still not been set
    // It should press the button again
    if (!buttonPressed[0]) {
        return 1;
    }

    sleep(BUTTON_HOLD_TIME * 2);
    ts.update(0, true);

    // Button release
    if (buttonPressed[0]) {
        return 1;
    }

    sleep(BUTTON_SYNC_TIME);
    ts.update(0x0 | FLAG_HARDPOINTS, true);

    // Wait for the sync time and this time tell the switch the flag has been set
    // It should not do anything since it's in sync with the game
    if (buttonPressed[0]) {
        return 1;
    }

    return 0;
}