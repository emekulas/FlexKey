#include "FlexKey_RFID.h"

FlexKeyRFID::FlexKeyRFID() 
    : nfc(nullptr), initialized(false), lastReadTime(0) {
}

bool FlexKeyRFID::begin() {
    // Initialize I2C
    Wire.begin(PIN_I2C_SDA, PIN_I2C_SCL);
    Serial.println("[RFID] I2C initialized (SDA=" + String(PIN_I2C_SDA) + ", SCL=" + String(PIN_I2C_SCL) + ")");
    
    // Create PN532 instance for I2C
    nfc = new Adafruit_PN532(PIN_I2C_SDA, PIN_I2C_SCL);
    
    if (!nfc) {
        Serial.println("[RFID] Failed to create PN532 instance");
        return false;
    }
    
    // Initialize PN532
    nfc->begin();
    
    // Check for PN532 board
    uint32_t versiondata = nfc->getFirmwareVersion();
    if (!versiondata) {
        Serial.println("[RFID] PN532 board not found");
        Serial.println("[RFID] Please check wiring:");
        Serial.println("[RFID]   SDA -> Pin " + String(PIN_I2C_SDA) + " (D4)");
        Serial.println("[RFID]   SCL -> Pin " + String(PIN_I2C_SCL) + " (D5)");
        Serial.println("[RFID]   VCC -> 3.3V");
        Serial.println("[RFID]   GND -> GND");
        delete nfc;
        nfc = nullptr;
        return false;
    }
    
    // Print firmware version
    Serial.print("[RFID] Found PN532 chip - Firmware ver. ");
    Serial.print((versiondata >> 24) & 0xFF, DEC);
    Serial.print('.');
    Serial.println((versiondata >> 16) & 0xFF, DEC);
    
    // Configure PN532 to read RFID tags
    nfc->SAMConfig();
    
    initialized = true;
    Serial.println("[RFID] PN532 initialized successfully");
    return true;
}

bool FlexKeyRFID::readCard(UID_t& uid) {
    if (!initialized || !nfc) {
        return false;
    }
    
    // Prevent rapid re-reads
    if (millis() - lastReadTime < READ_DELAY) {
        return false;
    }
    
    uint8_t success;
    uint8_t uidBuffer[7];
    uint8_t uidLength;
    
    // Try to read a card (timeout 100ms for non-blocking)
    success = nfc->readPassiveTargetID(PN532_MIFARE_ISO14443A, uidBuffer, &uidLength, 100);
    
    if (success) {
        // Validate UID length
        if (uidLength != 4 && uidLength != 7) {
            Serial.println("[RFID] Invalid UID length: " + String(uidLength));
            return false;
        }
        
        // Check if this is the same card as last read (prevent duplicate triggers)
        UID_t newUID;
        newUID.length = uidLength;
        memcpy(newUID.data, uidBuffer, uidLength);
        newUID.isValid = true;
        
        if (newUID.equals(lastReadUID)) {
            return false;  // Same card, ignore
        }
        
        // New card detected
        lastReadUID = newUID;
        lastReadTime = millis();
        uid = newUID;
        
        Serial.println("[RFID] Card detected!");
        Serial.println("[RFID] UID: " + uid.toString());
        Serial.println("[RFID] Length: " + String(uid.length) + " bytes");
        
        return true;
    }
    
    // No card detected - clear last read after delay
    if (millis() - lastReadTime > 2000) {
        lastReadUID.isValid = false;
    }
    
    return false;
}

void FlexKeyRFID::update() {
    // Can be used for periodic maintenance if needed
    // Currently handled in readCard()
}

bool FlexKeyRFID::isReady() {
    return initialized && (nfc != nullptr);
}
