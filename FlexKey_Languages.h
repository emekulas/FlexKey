#ifndef FLEXKEY_LANGUAGES_H
#define FLEXKEY_LANGUAGES_H

#include <Arduino.h>

// Dil metinleri için yapı
struct LanguageTexts {
    // Temel sistem metinleri
    String deviceId;
    String network;
    String status;
    String ready;
    String ssid;
    String gateway;
    String subnet;
    String ipAddress;
    String update;
    String active;
    String inactive;
    String open;
    String secure;
    String back;
    
    // Ana sistem metinleri
    String welcome;
    String systemAccess;
    String browserPassword;
    String enterSystem;
    String firstSetupTitle;
    String firstSetupDesc;
    String createPassword;
    String wifiConnection;
    String scanNetworks;
    String staticIP;
    String completeSetup;
    String mainTitle;
    String flexkeySettings;
    String deviceSettings;
    String info;
    String systemInformation;
    String currentNetwork;
    String systemVersion;
    String freeMemory;
    String restart;
    String factoryReset;
    String language;
    String rfidManagement;
    String httpTriggers;
    String flexkey2Required;
    String installFlexkey2;
    String configureHttpEndpoints;
    String browserPasswordLabel;
    String wifiSettings;
    String password;
    String updateWifi;
    String systemControls;
    String apNetwork;
    String restartSystemSafely;
    String resetAllUserData;
    String confirmRestart;
    String confirmFactoryReset;
    String passwordTooShort;
    String setupFailed;
    String setupCompleted;
    String noPasswordProvided;
    String passwordUpdated;
    String passwordUpdateFailed;
    String wifiUpdated;
    String wifiUpdateFailed;
    String unauthorized;
    String invalidData;
    String languageChanged;
    String invalidLanguage;
    String scanNetworksBtn;
    String showPassword;
    String hidePassword;
    String requiredField;
    String freeMemoryUnit;
    String passwordProtectionInfo;
    String apStatus;
    String wifiSettingsLabel;
    String apModeLabel;
    String systemControlsLabel;
    
    // FlexKey ayarları sayfası için ek metinler
    String relayDefaultSettings;
    String addNewGroup;
    String groupName;
    String add;
    String groups;
    String editGroup;
    String deleteGroup;
    String groupSettings;
    String defaultRelayState;
    String onTime;
    String offTime;
    String save;
    String cancel;
    String delete_;  // "delete" C++ keyword olduğu için delete_ kullanıyoruz
    
    // Açıklama metinleri
    String systemDescription;
    String connectionInfo;
    String detailedGuide;
    String restartInfo;
    String factoryResetInfo;
};

class FlexKeyLanguages {
public:
    FlexKeyLanguages();
    
    // Dil yönetimi
    void setLanguage(const String& lang);
    String getCurrentLanguage();
    LanguageTexts getTexts();
    
    // Çeviri fonksiyonları
    void loadLanguageTexts();
    String getTranslationJSON();
    
private:
    String currentLanguage;
    LanguageTexts texts;
    
    // Dil yükleme fonksiyonları
    void loadTurkishTexts();
    void loadGermanTexts();
    void loadEnglishTexts();
};

#endif // FLEXKEY_LANGUAGES_H
