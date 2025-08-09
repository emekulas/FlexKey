#ifndef FLEXKEY_WEB_H
#define FLEXKEY_WEB_H

#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <ArduinoJson.h>
#include "FlexKey_Languages.h"

// Forward declaration for FlexKey class
class FlexKey;

class FlexKeyWeb {
private:
    FlexKey* flexkey;  // Pointer to main FlexKey instance
    WebServer* server; // Pointer to WebServer instance
    FlexKeyLanguages* languageManager; // Pointer to language manager

public:
    FlexKeyWeb(FlexKey* fk, WebServer* srv, FlexKeyLanguages* langMgr);
    
    // Initialization
    void begin();
    
    // Web endpoint setup
    void setupWebRoutes();
    
    // Web handlers - Authentication & Main Pages
    void handleRoot();
    void handleLogin();
    void handleSetup();
    void handleFirstSetup();
    void handleCompleteSetup();
    void handleMainMenu();
    void handleDeviceSettings();
    void handleInfo();
    
    // Fragment handlers (for dynamic content)
    void handleDeviceSettingsFragment();
    void handleInfoFragment();
    
    // Style and assets
    void handleCSS();
    
    // Authentication and system
    void handleAuth();
    void handleRestart();
    void handleFactoryReset();
    
    // Language management
    void handleLanguageChange();
    void handleGetTranslations();
    
    // Settings management
    void handlePasswordUpdate();
    void handleWifiScan();
    void handleWifiUpdate();
    void handleToggleAP();
    
    // Helper methods
    bool checkAuth();
    String getChipID();
    String getAPSSID();
    String getBrowserPassword();
    bool getAPEnabled();
    bool getIsAuthenticated();
    bool getIsFirstSetup();
    
    // FlexKey_RF helper delegations
    String fx2NormalizeUid(const String &raw);
    bool fx2LoadConfig(DynamicJsonDocument &doc);
    bool fx2SaveConfig(DynamicJsonDocument &doc);
    void fx2ComputeStats(const DynamicJsonDocument &doc, int &gCount, int &uCount, int &urlCount);
    
private:
    // CSS generation helper
    String generateCSS();
    
    // HTML generation helpers
    String generateLoginHTML();
    String generateSetupHTML();
    String generateMainMenuHTML();
    String generateDeviceSettingsHTML();
    String generateInfoHTML();
};

#endif // FLEXKEY_WEB_H
