#include "FlexKey_Languages.h"

FlexKeyLanguages::FlexKeyLanguages() {
    currentLanguage = "en";
    loadLanguageTexts();
}

void FlexKeyLanguages::setLanguage(const String& lang) {
    if (lang == "en" || lang == "de" || lang == "tr") {
        currentLanguage = lang;
        loadLanguageTexts();
    }
}

String FlexKeyLanguages::getCurrentLanguage() {
    return currentLanguage;
}

LanguageTexts FlexKeyLanguages::getTexts() {
    return texts;
}

void FlexKeyLanguages::loadLanguageTexts() {
    // Ortak metinler (dil bağımsız)
    texts.deviceId = (currentLanguage == "tr") ? "Cihaz ID" : (currentLanguage == "de") ? "Geräte-ID" : "Device ID";
    texts.network = (currentLanguage == "tr") ? "Ağ" : (currentLanguage == "de") ? "Netzwerk" : "Network";
    texts.status = (currentLanguage == "tr") ? "Durum" : (currentLanguage == "de") ? "Status" : "Status";
    texts.ready = (currentLanguage == "tr") ? "Hazır" : (currentLanguage == "de") ? "Bereit" : "Ready";
    texts.ssid = "SSID";
    texts.gateway = (currentLanguage == "tr") ? "Ağ Geçidi" : (currentLanguage == "de") ? "Gateway" : "Gateway";
    texts.subnet = (currentLanguage == "tr") ? "Alt Ağ" : (currentLanguage == "de") ? "Subnetz" : "Subnet";
    texts.ipAddress = (currentLanguage == "tr") ? "IP Adresi" : (currentLanguage == "de") ? "IP-Adresse" : "IP Address";
    texts.update = (currentLanguage == "tr") ? "Güncelle" : (currentLanguage == "de") ? "Aktualisieren" : "Update";
    texts.active = (currentLanguage == "tr") ? "Aktif" : (currentLanguage == "de") ? "Aktiv" : "Active";
    texts.inactive = (currentLanguage == "tr") ? "İnaktif" : (currentLanguage == "de") ? "Inaktiv" : "Inactive";
    texts.open = (currentLanguage == "tr") ? "Açık" : (currentLanguage == "de") ? "Offen" : "Open";
    texts.secure = (currentLanguage == "tr") ? "Güvenli" : (currentLanguage == "de") ? "Sicher" : "Secure";
    texts.back = (currentLanguage == "tr") ? "Geri" : (currentLanguage == "de") ? "Zurück" : "Back";

    // Dil özelinde metinleri yükle
    if (currentLanguage == "tr") {
        loadTurkishTexts();
    } else if (currentLanguage == "de") {
        loadGermanTexts();
    } else {
        loadEnglishTexts();
    }
}

void FlexKeyLanguages::loadTurkishTexts() {
    texts.welcome = "Hoşgeldiniz";
    texts.systemAccess = "Sistem Giriş";
    texts.browserPassword = "Tarayıcı Şifresi:";
    texts.enterSystem = "Sisteme Gir";
    texts.firstSetupTitle = "İlk Kurulum";
    texts.firstSetupDesc = "FlexKey sisteminizi yapılandıralım";
    texts.createPassword = "Tarayıcı Şifresi Oluştur";
    texts.wifiConnection = "WiFi Bağlantısı";
    texts.scanNetworks = "Ağları Tara";
    texts.staticIP = "Statik IP Ayarları";
    texts.completeSetup = "Kurulumu Tamamla";
    texts.mainTitle = "FlexKey Sistemi";
    texts.flexkeySettings = "FlexKey Ayarları";
    texts.deviceSettings = "Cihaz Ayarları";
    texts.info = "Bilgi";
    texts.systemInformation = "Sistem Bilgisi";
    texts.currentNetwork = "Mevcut Ağ";
    texts.systemVersion = "Sistem Sürümü";
    texts.freeMemory = "Boş Bellek";
    texts.restart = "Yeniden Başlat";
    texts.factoryReset = "Fabrika Ayarları";
    texts.language = "Dil";
    texts.rfidManagement = "RFID Yönetimi";
    texts.httpTriggers = "HTTP Tetikleyiciler";
    texts.flexkey2Required = "FlexKey_2 Gerekli";
    texts.installFlexkey2 = "RFID kartlarını ve tetikleyicileri yönetmek için FlexKey_2 kütüphanesini yükleyin";
    texts.configureHttpEndpoints = "RFID tetikleyiciler için HTTP uç noktalarını yapılandırın";
    texts.browserPasswordLabel = "Tarayıcı Şifresi";
    texts.wifiSettings = "WiFi Ayarları";
    texts.password = "Şifre";
    texts.updateWifi = "WiFi'yi Güncelle";
    texts.systemControls = "Sistem Kontrolleri";
    texts.apNetwork = "AP Ağı";
    texts.restartSystemSafely = "Sistemi güvenli şekilde yeniden başlat";
    texts.resetAllUserData = "Tüm kullanıcı verilerini fabrika ayarlarına sıfırla";
    texts.confirmRestart = "Sistemi yeniden başlatmak istiyor musunuz?";
    texts.confirmFactoryReset = "Fabrika ayarlarına sıfırlama tüm kullanıcı verilerini silecek. Devam edilsin mi?";
    texts.passwordTooShort = "Şifre en az 4 karakter olmalı";
    texts.setupFailed = "Kurulum başarısız";
    texts.setupCompleted = "Kurulum tamamlandı";
    texts.noPasswordProvided = "Şifre girilmedi";
    texts.passwordUpdated = "Şifre başarıyla güncellendi";
    texts.passwordUpdateFailed = "Şifre güncellenemedi";
    texts.wifiUpdated = "WiFi ayarları güncellendi. Yeniden başlatmanız gerekebilir.";
    texts.wifiUpdateFailed = "WiFi ayarları kaydedilemedi";
    texts.unauthorized = "Yetkisiz erişim";
    texts.invalidData = "Geçersiz veri";
    texts.languageChanged = "Dil değiştirildi";
    texts.invalidLanguage = "Geçersiz dil";
    texts.scanNetworksBtn = "Ağları Tara";
    texts.showPassword = "Şifreyi Göster";
    texts.hidePassword = "Şifreyi Gizle";
    texts.requiredField = "Bu alan zorunludur";
    texts.freeMemoryUnit = "bayt";
    texts.passwordProtectionInfo = "Şifreniz hash değeri ile korunmaktadır. Yeni şifreniz de hash olarak kaydedilip saklanacaktır.";
    texts.apStatus = "AP Durumu";
    texts.wifiSettingsLabel = "WiFi Ayarları";
    texts.apModeLabel = "AP Modu";
    texts.systemControlsLabel = "Sistem Kontrolleri";
    
    // FlexKey ayarları sayfası için ek metinler
    texts.relayDefaultSettings = "Röle Varsayılan Ayarları";
    texts.addNewGroup = "Yeni Grup Ekle";
    texts.groupName = "Grup Adı";
    texts.add = "Ekle";
    texts.groups = "Gruplar";
    texts.editGroup = "Grubu Düzenle";
    texts.deleteGroup = "Grubu Sil";
    texts.groupSettings = "Grup Ayarları";
    texts.defaultRelayState = "Varsayılan Röle Durumu";
    texts.onTime = "Açık Kalma Süresi";
    texts.offTime = "Kapalı Kalma Süresi";
    texts.save = "Kaydet";
    texts.cancel = "İptal";
    texts.delete_ = "Sil";
    
    texts.systemDescription = "FlexKey, Gruplar ve Senaryolar ile ayarlanabilen RFID 13.56MHz frekansı ile çalışan bir anahtar sistemidir. Okutulan RFID ile Gruplara GET isteği atabileceğiniz gibi Röle ayarlaması ile fiziki işlemler de yapabilirsiniz.";
    texts.connectionInfo = "İlk kurulumdan sonra internet erişimi için isterseniz kendi WiFi ağınıza bağlayıp AP modunu kapatabilirsiniz.";
    texts.detailedGuide = "Detaylı kurulum ve kullanım kılavuzu için <a href='https://www.smartkraft.ch' target='_blank' style='color:#00ff88;text-decoration:underline;font-weight:bold;'><b>SmartKraft.ch</b></a> adresindeki ürün talimatlarına bakabilirsiniz.";
    texts.restartInfo = "Yeniden başlatmak için, cihazın arkasındaki butona <b>1 saniye</b> basın ya da <b>[FF:FF:FF:F1]</b> RFID kartınızı okutun.";
    texts.factoryResetInfo = "FlexKey cihazınızı ilk kurulum ayarlarına geri almak için, cihazın arkasındaki butona <b>10 saniye</b> basılı tutun ya da <b>[FF:FF:FF:F0]</b> RFID kartınızı okutun.";
}

void FlexKeyLanguages::loadGermanTexts() {
    texts.welcome = "Willkommen";
    texts.systemAccess = "Systemzugang";
    texts.browserPassword = "Browser-Passwort:";
    texts.enterSystem = "System betreten";
    texts.firstSetupTitle = "Ersteinrichtung";
    texts.firstSetupDesc = "Lassen Sie uns Ihr FlexKey-System konfigurieren";
    texts.createPassword = "Browser-Passwort erstellen";
    texts.wifiConnection = "WiFi-Verbindung";
    texts.scanNetworks = "Netzwerke scannen";
    texts.staticIP = "Statische IP-Einstellungen";
    texts.completeSetup = "Setup abschließen";
    texts.mainTitle = "FlexKey System";
    texts.flexkeySettings = "FlexKey-Einstellungen";
    texts.deviceSettings = "Geräteeinstellungen";
    texts.info = "Info";
    texts.systemInformation = "Systeminformation";
    texts.currentNetwork = "Aktuelles Netzwerk";
    texts.systemVersion = "Systemversion";
    texts.freeMemory = "Freier Speicher";
    texts.restart = "Neustart";
    texts.factoryReset = "Werkseinstellungen";
    texts.language = "Sprache";
    texts.rfidManagement = "RFID-Verwaltung";
    texts.httpTriggers = "HTTP-Trigger";
    texts.flexkey2Required = "FlexKey_2 erforderlich";
    texts.installFlexkey2 = "Installieren Sie die FlexKey_2-Bibliothek zur Verwaltung von RFID-Karten und Triggern";
    texts.configureHttpEndpoints = "HTTP-Endpunkte für RFID-Trigger konfigurieren";
    texts.browserPasswordLabel = "Browser-Passwort";
    texts.wifiSettings = "WiFi-Einstellungen";
    texts.password = "Passwort";
    texts.updateWifi = "WiFi aktualisieren";
    texts.systemControls = "Systemsteuerung";
    texts.apNetwork = "AP-Netzwerk";
    texts.restartSystemSafely = "System sicher neu starten";
    texts.resetAllUserData = "Alle Benutzerdaten auf Werkseinstellungen zurücksetzen";
    texts.confirmRestart = "Möchten Sie das System neu starten?";
    texts.confirmFactoryReset = "Das Zurücksetzen auf Werkseinstellungen löscht alle Benutzerdaten. Fortfahren?";
    texts.passwordTooShort = "Passwort muss mindestens 4 Zeichen lang sein";
    texts.setupFailed = "Setup fehlgeschlagen";
    texts.setupCompleted = "Setup abgeschlossen";
    texts.noPasswordProvided = "Kein Passwort angegeben";
    texts.passwordUpdated = "Passwort erfolgreich aktualisiert";
    texts.passwordUpdateFailed = "Passwort konnte nicht aktualisiert werden";
    texts.wifiUpdated = "WiFi-Einstellungen aktualisiert. Möglicherweise müssen Sie neu starten.";
    texts.wifiUpdateFailed = "WiFi-Einstellungen konnten nicht gespeichert werden";
    texts.unauthorized = "Unbefugter Zugriff";
    texts.invalidData = "Ungültige Daten";
    texts.languageChanged = "Sprache geändert";
    texts.invalidLanguage = "Ungültige Sprache";
    texts.scanNetworksBtn = "Netzwerke scannen";
    texts.showPassword = "Passwort anzeigen";
    texts.hidePassword = "Passwort verbergen";
    texts.requiredField = "Dieses Feld ist erforderlich";
    texts.freeMemoryUnit = "Bytes";
    texts.passwordProtectionInfo = "Ihr Passwort wird mit einem Hash-Wert geschützt. Ihr neues Passwort wird ebenfalls gehasht gespeichert.";
    texts.apStatus = "AP-Status";
    texts.wifiSettingsLabel = "WiFi-Einstellungen";
    texts.apModeLabel = "AP-Modus";
    texts.systemControlsLabel = "Systemsteuerung";
    
    // FlexKey ayarları sayfası için ek metinler
    texts.relayDefaultSettings = "Relais-Standardeinstellungen";
    texts.addNewGroup = "Neue Gruppe hinzufügen";
    texts.groupName = "Gruppenname";
    texts.add = "Hinzufügen";
    texts.groups = "Gruppen";
    texts.editGroup = "Gruppe bearbeiten";
    texts.deleteGroup = "Gruppe löschen";
    texts.groupSettings = "Gruppeneinstellungen";
    texts.defaultRelayState = "Standard-Relaiszustand";
    texts.onTime = "Einschaltdauer";
    texts.offTime = "Ausschaltdauer";
    texts.save = "Speichern";
    texts.cancel = "Abbrechen";
    texts.delete_ = "Löschen";
    
    texts.systemDescription = "FlexKey ist ein Schlüsselsystem, das mit RFID 13.56MHz-Frequenz arbeitet und mit Gruppen und Szenarien konfiguriert werden kann. Mit gelesenen RFID-Karten können Sie GET-Anfragen an Gruppen senden oder physische Operationen mit Relais-Einstellungen durchführen.";
    texts.connectionInfo = "Nach der ersten Einrichtung können Sie für den Internetzugang eine Verbindung zu Ihrem eigenen WiFi-Netzwerk herstellen und den AP-Modus deaktivieren.";
    texts.detailedGuide = "Für eine detaillierte Installations- und Bedienungsanleitung besuchen Sie die Produktanweisungen auf <a href='https://www.smartkraft.ch' target='_blank' style='color:#00ff88;text-decoration:underline;font-weight:bold;'><b>SmartKraft.ch</b></a>.";
    texts.restartInfo = "Zum Neustart drücken Sie die Taste auf der Rückseite des Geräts für <b>1 Sekunde</b> oder lesen Sie die RFID-Karte <b>[FF:FF:FF:F1]</b>.";
    texts.factoryResetInfo = "Um Ihr FlexKey-Gerät auf die ursprünglichen Einstellungen zurückzusetzen, halten Sie die Taste auf der Rückseite des Geräts <b>10 Sekunden</b> gedrückt oder lesen Sie die RFID-Karte <b>[FF:FF:FF:F0]</b>.";
}

void FlexKeyLanguages::loadEnglishTexts() {
    texts.welcome = "Welcome";
    texts.systemAccess = "System Access";
    texts.browserPassword = "Browser Password:";
    texts.enterSystem = "Enter System";
    texts.firstSetupTitle = "First Setup";
    texts.firstSetupDesc = "Let's configure your FlexKey system";
    texts.createPassword = "Create Browser Password";
    texts.wifiConnection = "WiFi Connection";
    texts.scanNetworks = "Scan Networks";
    texts.staticIP = "Static IP Settings";
    texts.completeSetup = "Complete Setup";
    texts.mainTitle = "FlexKey System";
    texts.flexkeySettings = "FlexKey Settings";
    texts.deviceSettings = "Device Settings";
    texts.info = "Info";
    texts.systemInformation = "System Information";
    texts.currentNetwork = "Current Network";
    texts.systemVersion = "System Version";
    texts.freeMemory = "Free Memory";
    texts.restart = "Restart";
    texts.factoryReset = "Factory Reset";
    texts.language = "Language";
    texts.rfidManagement = "RFID Management";
    texts.httpTriggers = "HTTP Triggers";
    texts.flexkey2Required = "FlexKey_2 Required";
    texts.installFlexkey2 = "Install FlexKey_2 library to manage RFID cards and triggers";
    texts.configureHttpEndpoints = "Configure HTTP endpoints for RFID triggers";
    texts.browserPasswordLabel = "Browser Password";
    texts.wifiSettings = "WiFi Settings";
    texts.password = "Password";
    texts.updateWifi = "Update WiFi";
    texts.systemControls = "System Controls";
    texts.apNetwork = "AP Network";
    texts.restartSystemSafely = "Restart system safely";
    texts.resetAllUserData = "Reset all user data to factory settings";
    texts.confirmRestart = "Do you want to restart the system?";
    texts.confirmFactoryReset = "Factory reset will delete all user data. Continue?";
    texts.passwordTooShort = "Password must be at least 4 characters";
    texts.setupFailed = "Setup failed";
    texts.setupCompleted = "Setup completed";
    texts.noPasswordProvided = "No password provided";
    texts.passwordUpdated = "Password updated successfully";
    texts.passwordUpdateFailed = "Password could not be updated";
    texts.wifiUpdated = "WiFi settings updated. You may need to restart.";
    texts.wifiUpdateFailed = "WiFi settings could not be saved";
    texts.unauthorized = "Unauthorized access";
    texts.invalidData = "Invalid data";
    texts.languageChanged = "Language changed";
    texts.invalidLanguage = "Invalid language";
    texts.scanNetworksBtn = "Scan Networks";
    texts.showPassword = "Show Password";
    texts.hidePassword = "Hide Password";
    texts.requiredField = "This field is required";
    texts.freeMemoryUnit = "bytes";
    texts.passwordProtectionInfo = "Your password is protected with a hash value. Your new password will also be stored hashed.";
    texts.apStatus = "AP Status";
    texts.wifiSettingsLabel = "WiFi Settings";
    texts.apModeLabel = "AP Mode";
    texts.systemControlsLabel = "System Controls";
    
    // FlexKey ayarları sayfası için ek metinler
    texts.relayDefaultSettings = "Relay Default Settings";
    texts.addNewGroup = "Add New Group";
    texts.groupName = "Group Name";
    texts.add = "Add";
    texts.groups = "Groups";
    texts.editGroup = "Edit Group";
    texts.deleteGroup = "Delete Group";
    texts.groupSettings = "Group Settings";
    texts.defaultRelayState = "Default Relay State";
    texts.onTime = "On Time";
    texts.offTime = "Off Time";
    texts.save = "Save";
    texts.cancel = "Cancel";
    texts.delete_ = "Delete";
    
    texts.systemDescription = "FlexKey is a key system that works with RFID 13.56MHz frequency and can be configured with Groups and Scenarios. With read RFID cards, you can send GET requests to Groups as well as perform physical operations with Relay settings.";
    texts.connectionInfo = "After the initial setup, you can connect to your own WiFi network for internet access and turn off AP mode if desired.";
    texts.detailedGuide = "For detailed installation and usage guide, you can check the product instructions at <a href='https://www.smartkraft.ch' target='_blank' style='color:#00ff88;text-decoration:underline;font-weight:bold;'><b>SmartKraft.ch</b></a>.";
    texts.restartInfo = "To restart, press the button on the back of the device for <b>1 second</b> or read the RFID card <b>[FF:FF:FF:F1]</b>.";
    texts.factoryResetInfo = "To reset your FlexKey device to initial setup settings, hold the button on the back of the device for <b>10 seconds</b> or read the RFID card <b>[FF:FF:FF:F0]</b>.";
}

String FlexKeyLanguages::getTranslationJSON() {
    String json = "{";
    json += "\"deviceId\":\"" + texts.deviceId + "\",";
    json += "\"network\":\"" + texts.network + "\",";
    json += "\"status\":\"" + texts.status + "\",";
    json += "\"ready\":\"" + texts.ready + "\",";
    json += "\"ssid\":\"" + texts.ssid + "\",";
    json += "\"gateway\":\"" + texts.gateway + "\",";
    json += "\"subnet\":\"" + texts.subnet + "\",";
    json += "\"ipAddress\":\"" + texts.ipAddress + "\",";
    json += "\"update\":\"" + texts.update + "\",";
    json += "\"active\":\"" + texts.active + "\",";
    json += "\"inactive\":\"" + texts.inactive + "\",";
    json += "\"open\":\"" + texts.open + "\",";
    json += "\"secure\":\"" + texts.secure + "\",";
    json += "\"back\":\"" + texts.back + "\",";
    json += "\"welcome\":\"" + texts.welcome + "\",";
    json += "\"systemAccess\":\"" + texts.systemAccess + "\",";
    json += "\"browserPassword\":\"" + texts.browserPassword + "\",";
    json += "\"enterSystem\":\"" + texts.enterSystem + "\",";
    json += "\"firstSetupTitle\":\"" + texts.firstSetupTitle + "\",";
    json += "\"firstSetupDesc\":\"" + texts.firstSetupDesc + "\",";
    json += "\"createPassword\":\"" + texts.createPassword + "\",";
    json += "\"wifiConnection\":\"" + texts.wifiConnection + "\",";
    json += "\"scanNetworks\":\"" + texts.scanNetworks + "\",";
    json += "\"staticIP\":\"" + texts.staticIP + "\",";
    json += "\"completeSetup\":\"" + texts.completeSetup + "\",";
    json += "\"mainTitle\":\"" + texts.mainTitle + "\",";
    json += "\"flexkeySettings\":\"" + texts.flexkeySettings + "\",";
    json += "\"deviceSettings\":\"" + texts.deviceSettings + "\",";
    json += "\"info\":\"" + texts.info + "\",";
    json += "\"systemInformation\":\"" + texts.systemInformation + "\",";
    json += "\"currentNetwork\":\"" + texts.currentNetwork + "\",";
    json += "\"systemVersion\":\"" + texts.systemVersion + "\",";
    json += "\"freeMemory\":\"" + texts.freeMemory + "\",";
    json += "\"restart\":\"" + texts.restart + "\",";
    json += "\"factoryReset\":\"" + texts.factoryReset + "\",";
    json += "\"language\":\"" + texts.language + "\",";
    json += "\"rfidManagement\":\"" + texts.rfidManagement + "\",";
    json += "\"httpTriggers\":\"" + texts.httpTriggers + "\",";
    json += "\"flexkey2Required\":\"" + texts.flexkey2Required + "\",";
    json += "\"installFlexkey2\":\"" + texts.installFlexkey2 + "\",";
    json += "\"configureHttpEndpoints\":\"" + texts.configureHttpEndpoints + "\",";
    json += "\"browserPasswordLabel\":\"" + texts.browserPasswordLabel + "\",";
    json += "\"wifiSettings\":\"" + texts.wifiSettings + "\",";
    json += "\"password\":\"" + texts.password + "\",";
    json += "\"updateWifi\":\"" + texts.updateWifi + "\",";
    json += "\"systemControls\":\"" + texts.systemControls + "\",";
    json += "\"apNetwork\":\"" + texts.apNetwork + "\",";
    json += "\"restartSystemSafely\":\"" + texts.restartSystemSafely + "\",";
    json += "\"resetAllUserData\":\"" + texts.resetAllUserData + "\",";
    json += "\"confirmRestart\":\"" + texts.confirmRestart + "\",";
    json += "\"confirmFactoryReset\":\"" + texts.confirmFactoryReset + "\",";
    json += "\"passwordTooShort\":\"" + texts.passwordTooShort + "\",";
    json += "\"setupFailed\":\"" + texts.setupFailed + "\",";
    json += "\"setupCompleted\":\"" + texts.setupCompleted + "\",";
    json += "\"noPasswordProvided\":\"" + texts.noPasswordProvided + "\",";
    json += "\"passwordUpdated\":\"" + texts.passwordUpdated + "\",";
    json += "\"passwordUpdateFailed\":\"" + texts.passwordUpdateFailed + "\",";
    json += "\"wifiUpdated\":\"" + texts.wifiUpdated + "\",";
    json += "\"wifiUpdateFailed\":\"" + texts.wifiUpdateFailed + "\",";
    json += "\"unauthorized\":\"" + texts.unauthorized + "\",";
    json += "\"invalidData\":\"" + texts.invalidData + "\",";
    json += "\"languageChanged\":\"" + texts.languageChanged + "\",";
    json += "\"invalidLanguage\":\"" + texts.invalidLanguage + "\",";
    json += "\"scanNetworksBtn\":\"" + texts.scanNetworksBtn + "\",";
    json += "\"showPassword\":\"" + texts.showPassword + "\",";
    json += "\"hidePassword\":\"" + texts.hidePassword + "\",";
    json += "\"requiredField\":\"" + texts.requiredField + "\",";
    json += "\"freeMemoryUnit\":\"" + texts.freeMemoryUnit + "\",";
    json += "\"passwordProtectionInfo\":\"" + texts.passwordProtectionInfo + "\",";
    json += "\"apStatus\":\"" + texts.apStatus + "\",";
    json += "\"wifiSettingsLabel\":\"" + texts.wifiSettingsLabel + "\",";
    json += "\"apModeLabel\":\"" + texts.apModeLabel + "\",";
    json += "\"systemControlsLabel\":\"" + texts.systemControlsLabel + "\",";
    json += "\"systemDescription\":\"" + texts.systemDescription + "\",";
    json += "\"connectionInfo\":\"" + texts.connectionInfo + "\",";
    json += "\"detailedGuide\":\"" + texts.detailedGuide + "\",";
    json += "\"restartInfo\":\"" + texts.restartInfo + "\",";
    json += "\"factoryResetInfo\":\"" + texts.factoryResetInfo + "\"";
    json += "}";
    return json;
}
