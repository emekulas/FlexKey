#include "FlexKey_Button.h"

FlexKeyButton::FlexKeyButton() 
    : pressStartTime(0), lastDebounceTime(0), 
      buttonState(HIGH), lastButtonState(HIGH), 
      factoryResetTriggered(false) {
}

void FlexKeyButton::begin() {
    pinMode(PIN_BUTTON, INPUT_PULLUP);
    Serial.println("[BUTTON] Initialized on pin " + String(PIN_BUTTON));
}

bool FlexKeyButton::update() {
    int reading = digitalRead(PIN_BUTTON);
    
    // Debounce logic
    if (reading != lastButtonState) {
        lastDebounceTime = millis();
    }
    
    if ((millis() - lastDebounceTime) > BUTTON_DEBOUNCE_MS) {
        if (reading != buttonState) {
            buttonState = reading;
            
            // Button pressed (LOW because of pullup)
            if (buttonState == LOW) {
                pressStartTime = millis();
                Serial.println("[BUTTON] Pressed");
            }
            // Button released
            else {
                unsigned long holdDuration = millis() - pressStartTime;
                Serial.println("[BUTTON] Released (held for " + String(holdDuration) + "ms)");
                
                // Short press = restart
                if (holdDuration < FACTORY_RESET_HOLD_MS) {
                    Serial.println("[BUTTON] Restart triggered");
                    delay(500);
                    ESP.restart();
                }
                
                pressStartTime = 0;
            }
        }
        
        // Check for factory reset (10 seconds hold)
        if (buttonState == LOW && !factoryResetTriggered) {
            unsigned long holdDuration = millis() - pressStartTime;
            
            if (holdDuration >= FACTORY_RESET_HOLD_MS) {
                factoryResetTriggered = true;
                Serial.println("[BUTTON] ==========================================");
                Serial.println("[BUTTON] FACTORY RESET TRIGGERED!");
                Serial.println("[BUTTON] ==========================================");
                return true;  // Signal factory reset
            }
            
            // Progress indicator every second
            if (holdDuration > 0 && holdDuration % 1000 == 0) {
                Serial.println("[BUTTON] Hold progress: " + String(holdDuration / 1000) + "/10 seconds");
            }
        }
    }
    
    lastButtonState = reading;
    return false;
}

bool FlexKeyButton::isPressed() {
    return buttonState == LOW;
}

unsigned long FlexKeyButton::getHoldDuration() {
    if (buttonState == LOW && pressStartTime > 0) {
        return millis() - pressStartTime;
    }
    return 0;
}
