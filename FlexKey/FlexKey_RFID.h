#ifndef FLEXKEY_RFID_H
#define FLEXKEY_RFID_H

#include "FlexKey_Config.h"
#include <Wire.h>
#include <Adafruit_PN532.h>

class FlexKeyRFID {
public:
    FlexKeyRFID();
    
    // Initialize PN532
    bool begin();
    
    // Check for card and read UID
    bool readCard(UID_t& uid);
    
    // Update loop (call frequently)
    void update();
    
    // Check if PN532 is ready
    bool isReady();
    
private:
    Adafruit_PN532* nfc;
    bool initialized;
    unsigned long lastReadTime;
    UID_t lastReadUID;
    
    static const unsigned long READ_DELAY = 500;  // Min delay between reads (ms)
};

#endif // FLEXKEY_RFID_H
