#ifndef FLEXKEY_H
#define FLEXKEY_H

#include <WiFi.h>
#include <WebServer.h>
#include <SPIFFS.h>
#include <ArduinoJson.h>
#include <esp_system.h>
#include "FlexKey_Languages.h"

// Forward declaration
class FlexKeyWeb;

class FlexKey {
private:
    bool apEnabled; // AP modunun açık/kapalı durumu
    WebServer server;
    String chipID;
    String apSSID;
    String browserPassword;
    bool isAuthenticated;
    bool isFirstSetup;
    String currentLanguage;
    FlexKeyLanguages languageManager;
    FlexKeyWeb *webUI;  // Web UI modülü
    
    // System configuration paths (unchangeable)
    const char* SYSTEM_CONFIG_PATH = "/system/config.json";
    const char* FIRST_SETUP_FLAG = "/system/setup_complete.flag";
    const char* WEB_ASSETS_PATH = "/system/web/";
    const char* FXRF_CONFIG_PATH = "/user/fxrf.json";
    
    // Pin definitions
    const int BUTTON_PIN = 12;
    const int RELAY_PIN = 16;
    
    // Button state tracking
    unsigned long buttonPressTime = 0;
    bool buttonPressed = false;
    
    
    // Methods
    void initializePins();
    void initializeFileSystem();
    void generateChipID();
    void createSystemConfig();
    void setAPMode(bool enable);
    void setupWebServer();
    void handleButtonPress();
    void setLanguage(String lang);
    bool isFirstTimeSetup();
    void markSetupComplete();
    
    // FlexKey_RF helpers delegated to FlexKey_RF::Manager
    String fxrfNormalizeUid(const String &raw);
    bool fxrfLoadConfig(DynamicJsonDocument &doc);
    bool fxrfSaveConfig(DynamicJsonDocument &doc);
    void fxrfComputeStats(const DynamicJsonDocument &doc, int &gCount, int &uCount, int &urlCount);

public:
    FlexKey();
    void begin();
    void loop();
    void restart();
    void factoryReset();
    
    // Public getters and methods
    String getChipID();
    String getAPSSID();
    String getBrowserPassword();
    bool isSystemReady();
    bool getAPEnabled();
    bool getIsAuthenticated();
    bool getIsFirstSetup();
    bool validatePassword(const String& password);
    void handleCompleteSetup();
    bool checkAuth();
};

#endif
