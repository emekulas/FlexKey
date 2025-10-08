#ifndef FLEXKEY_BUTTON_H
#define FLEXKEY_BUTTON_H

#include "FlexKey_Config.h"

class FlexKeyButton {
public:
    FlexKeyButton();
    
    // Initialize button
    void begin();
    
    // Call this in loop() - returns true if factory reset triggered
    bool update();
    
    // Check if button is pressed
    bool isPressed();
    
    // Get hold duration in milliseconds
    unsigned long getHoldDuration();
    
private:
    unsigned long pressStartTime;
    unsigned long lastDebounceTime;
    bool buttonState;
    bool lastButtonState;
    bool factoryResetTriggered;
};

#endif // FLEXKEY_BUTTON_H
