#include "FlexKey_web.h"
#include "FlexKey.h"

FlexKeyWeb::FlexKeyWeb(FlexKey* fk, WebServer* srv, FlexKeyLanguages* langMgr) {
    flexkey = fk;
    server = srv;
    languageManager = langMgr;
}

void FlexKeyWeb::begin() {
    // Web module is ready
}

void FlexKeyWeb::setupWebRoutes() {
    if (!server) return;
    
    // Main pages
    server->on("/", HTTP_GET, [this]() { handleRoot(); });
    server->on("/login", HTTP_POST, [this]() { handleLogin(); });
    server->on("/setup", HTTP_GET, [this]() { handleSetup(); });
    server->on("/first-setup", HTTP_GET, [this]() { handleFirstSetup(); });
    server->on("/complete-setup", HTTP_POST, [this]() { handleCompleteSetup(); });
    server->on("/main-menu", HTTP_GET, [this]() { handleMainMenu(); });
    server->on("/device-settings", HTTP_GET, [this]() { handleDeviceSettings(); });
    server->on("/info", HTTP_GET, [this]() { handleInfo(); });
    
    // Assets and fragments
    server->on("/style.css", HTTP_GET, [this]() { handleCSS(); });
    server->on("/fragment/device-settings", HTTP_GET, [this]() { handleDeviceSettingsFragment(); });
    server->on("/fragment/info", HTTP_GET, [this]() { handleInfoFragment(); });
    
    // Authentication and system control
    server->on("/auth", HTTP_GET, [this]() { handleAuth(); });
    server->on("/restart", HTTP_POST, [this]() { handleRestart(); });
    server->on("/factory-reset", HTTP_POST, [this]() { handleFactoryReset(); });
    
    // Language and settings
    server->on("/set-language", HTTP_POST, [this]() { handleLanguageChange(); });
    server->on("/get-translations", HTTP_GET, [this]() { handleGetTranslations(); });
    server->on("/update-password", HTTP_POST, [this]() { handlePasswordUpdate(); });
    server->on("/wifi-scan", HTTP_GET, [this]() { handleWifiScan(); });
    server->on("/update-wifi", HTTP_POST, [this]() { handleWifiUpdate(); });
    server->on("/toggle-ap", HTTP_POST, [this]() { handleToggleAP(); });
}

// Helper methods to access FlexKey data
bool FlexKeyWeb::checkAuth() {
    return flexkey ? flexkey->checkAuth() : false;
}

String FlexKeyWeb::getChipID() {
    return flexkey ? flexkey->getChipID() : "";
}

String FlexKeyWeb::getAPSSID() {
    return flexkey ? flexkey->getAPSSID() : "";
}

String FlexKeyWeb::getBrowserPassword() {
    return flexkey ? flexkey->getBrowserPassword() : "";
}

bool FlexKeyWeb::getAPEnabled() {
    return flexkey ? flexkey->getAPEnabled() : false;
}

bool FlexKeyWeb::getIsAuthenticated() {
    return flexkey ? flexkey->getIsAuthenticated() : false;
}

bool FlexKeyWeb::getIsFirstSetup() {
    return flexkey ? flexkey->getIsFirstSetup() : true;
}

void FlexKeyWeb::handleRoot() {
    if (getIsFirstSetup()) {
        server->sendHeader("Location", "/first-setup");
        server->send(302, "text/plain", "");
        return;
    }
    
    if (!getIsAuthenticated()) {
        server->send(200, "text/html", generateLoginHTML());
        return;
    }
    
    server->sendHeader("Location", "/main-menu");
    server->send(302, "text/plain", "");
}

void FlexKeyWeb::handleLogin() {
    if (server->hasArg("password")) {
        String inputPassword = server->arg("password");
        if (flexkey && flexkey->validatePassword(inputPassword)) {
            server->send(200, "text/plain", "success");
        } else {
            server->send(401, "text/plain", "Invalid password");
        }
    } else {
        server->send(400, "text/plain", "No password provided");
    }
}

void FlexKeyWeb::handleSetup() {
    server->send(200, "text/html", generateSetupHTML());
}

void FlexKeyWeb::handleFirstSetup() {
    if (!getIsFirstSetup()) {
        server->sendHeader("Location", "/");
        server->send(302, "text/plain", "");
        return;
    }
    
    LanguageTexts texts = languageManager->getTexts();
    
    String html = "<!DOCTYPE html><html><head><meta charset='UTF-8'><meta name='viewport' content='width=device-width, initial-scale=1.0, user-scalable=no'><title>" + texts.firstSetupTitle + "</title><link rel='stylesheet' href='/style.css'></head><body>";
    html += "<div class='main-container'><div class='container'>";
    html += "<div class='top-header'><h1>FlexKey</h1><div class='subtitle'>" + texts.firstSetupDesc + "</div>";
    html += "<a href='https://www.smartkraft.ch' target='_blank' class='smartkraft'>SmartKraft.ch</a></div>";
    
    // Language selector
    html += "<div class='language-selector'>";
    html += "<button class='lang-btn active' onclick='setLanguage(\"tr\")'>Türkçe</button>";
    html += "<button class='lang-btn' onclick='setLanguage(\"en\")'>English</button>";
    html += "<button class='lang-btn' onclick='setLanguage(\"de\")'>Deutsch</button>";
    html += "</div>";
    
    html += "<div class='setup-wizard'>";
    html += "<h2>" + texts.firstSetupTitle + "</h2>";
    html += "<p style='margin-bottom:20px;color:#9ca3af;'>" + texts.firstSetupDesc + "</p>";
    
    // Password creation
    html += "<div class='form-group'>";
    html += "<label>" + texts.createPassword + ":</label>";
    html += "<input type='password' id='setupPassword' placeholder='" + texts.password + "' required>";
    html += "</div>";
    
    // WiFi section
    html += "<div class='form-group'>";
    html += "<label>" + texts.wifiConnection + ":</label>";
    html += "<button type='button' onclick='scanNetworks()' class='btn-secondary'>" + texts.scanNetworks + "</button>";
    html += "<div id='networkList' style='margin-top:10px;'></div>";
    html += "<input type='text' id='wifiSSID' placeholder='WiFi " + texts.ssid + "'>";
    html += "<input type='password' id='wifiPassword' placeholder='WiFi " + texts.password + "'>";
    html += "</div>";
    
    // Static IP toggle
    html += "<div class='toggle-switch'>";
    html += "<label>" + texts.staticIP + ":</label>";
    html += "<label><input type='checkbox' id='useStaticIP' onchange='toggleStaticIP()'><span class='switch'><span class='slider'></span></span></label>";
    html += "</div>";
    
    // Static IP fields (hidden by default)
    html += "<div id='staticIPFields' style='display:none;'>";
    html += "<div class='form-group'>";
    html += "<label>" + texts.ipAddress + ":</label>";
    html += "<input type='text' id='staticIP' placeholder='192.168.1.100'>";
    html += "</div>";
    html += "<div class='form-group'>";
    html += "<label>" + texts.gateway + ":</label>";
    html += "<input type='text' id='gateway' placeholder='192.168.1.1'>";
    html += "</div>";
    html += "<div class='form-group'>";
    html += "<label>" + texts.subnet + ":</label>";
    html += "<input type='text' id='subnet' placeholder='255.255.255.0'>";
    html += "</div>";
    html += "</div>";
    
    html += "<button onclick='completeSetup()' class='btn-primary' style='width:100%;margin-top:20px;'>" + texts.completeSetup + "</button>";
    html += "</div></div></div>";
    
    // JavaScript for setup functionality
    html += "<script>";
    html += "function scanNetworks(){fetch('/wifi-scan').then(r=>r.json()).then(networks=>{const list=document.getElementById('networkList');list.innerHTML='';networks.forEach(n=>{const btn=document.createElement('button');btn.className='btn-secondary';btn.style='margin:2px;font-size:12px;';btn.textContent=n.ssid+'('+n.rssi+'dBm)';btn.onclick=()=>{document.getElementById('wifiSSID').value=n.ssid;};list.appendChild(btn);});}).catch(()=>alert('Ağ taraması başarısız'));}";
    html += "function toggleStaticIP(){const fields=document.getElementById('staticIPFields');const checkbox=document.getElementById('useStaticIP');fields.style.display=checkbox.checked?'block':'none';}";
    html += "function setLanguage(lang){fetch('/set-language',{method:'POST',headers:{'Content-Type':'application/x-www-form-urlencoded'},body:'language='+lang}).then(()=>location.reload());}";
    html += "function completeSetup(){const password=document.getElementById('setupPassword').value;const ssid=document.getElementById('wifiSSID').value;const wifiPass=document.getElementById('wifiPassword').value;const useStatic=document.getElementById('useStaticIP').checked;let body='password='+encodeURIComponent(password)+'&ssid='+encodeURIComponent(ssid)+'&wifiPassword='+encodeURIComponent(wifiPass);if(useStatic){body+='&useStaticIP=true&staticIP='+encodeURIComponent(document.getElementById('staticIP').value)+'&gateway='+encodeURIComponent(document.getElementById('gateway').value)+'&subnet='+encodeURIComponent(document.getElementById('subnet').value);}fetch('/complete-setup',{method:'POST',headers:{'Content-Type':'application/x-www-form-urlencoded'},body:body}).then(r=>r.text()).then(result=>{if(result.includes('success')){alert('Kurulum tamamlandı!');location.href='/';}else{alert('Kurulum hatası: '+result);}}).catch(()=>alert('Kurulum başarısız'));}";
    html += "</script></body></html>";
    
    server->send(200, "text/html", html);
}

void FlexKeyWeb::handleCompleteSetup() {
    if (flexkey) {
        flexkey->handleCompleteSetup();
    } else {
        server->send(500, "text/plain", "System error");
    }
}

void FlexKeyWeb::handleMainMenu() {
    if (!checkAuth()) {
        server->sendHeader("Location", "/");
        server->send(302, "text/plain", "");
        return;
    }
    
    server->send(200, "text/html", generateMainMenuHTML());
}

void FlexKeyWeb::handleCSS() {
    server->sendHeader("Cache-Control", "public, max-age=86400");
    server->send(200, "text/css", generateCSS());
}

String FlexKeyWeb::generateCSS() {
    String css = "*{margin:0;padding:0;box-sizing:border-box;}";
    
    // Dark modern design inspired by SmartKraft
    css += "body{font-family:'Inter','Segoe UI',system-ui,-apple-system,sans-serif;background:#1a1a1a;color:#e5e7eb;min-height:100vh;overflow-x:hidden;}";
    
    // Containers with dark glass morphism effect
    css += ".main-container{min-height:100vh;display:flex;flex-direction:column;background:linear-gradient(135deg,rgba(0,0,0,0.8) 0%,rgba(26,26,26,0.9) 100%);}";
    css += ".container{max-width:700px;margin:0 auto;padding:20px;}";
    
    // Header with dark styling and green accent
    css += ".top-header{text-align:center;padding:32px 20px 24px 20px;background:rgba(26,26,26,0.95);backdrop-filter:blur(20px);border-radius:0 0 24px 24px;box-shadow:0 8px 32px rgba(0,0,0,0.3);border:1px solid rgba(255,255,255,0.1);}";
    css += ".top-header h1{font-size:2.5em;margin:0;color:#00ff88;font-weight:700;letter-spacing:-0.5px;text-shadow:0 0 20px rgba(0,255,136,0.3);}";
    css += ".top-header .subtitle{margin-top:8px;font-size:1.1em;color:#e5e7eb;font-weight:500;}";
    css += ".top-header .smartkraft{color:#e5e7eb;font-weight:600;cursor:pointer;transition:all 0.3s ease;text-decoration:none;}";
    css += ".top-header .smartkraft:hover{color:#00ff88;text-shadow:0 0 10px rgba(0,255,136,0.2);}";
    
    // Language selector with dark design
    css += ".language-selector{display:flex;gap:8px;justify-content:center;}";
    css += ".lang-btn{background:rgba(26,26,26,0.8);color:#d1d5db;border:1px solid rgba(255,255,255,0.1);padding:8px 16px;border-radius:12px;cursor:pointer;font-size:14px;font-weight:500;transition:all 0.3s cubic-bezier(0.4,0,0.2,1);backdrop-filter:blur(10px);}";
    css += ".lang-btn:hover{background:rgba(0,255,136,0.1);border-color:#00ff88;color:#00ff88;transform:translateY(-2px);box-shadow:0 4px 12px rgba(0,255,136,0.1);}";
    css += ".lang-btn.active{background:#00ff88;color:#1a1a1a;border-color:#00ff88;box-shadow:0 4px 16px rgba(0,255,136,0.3);}";
    
    // ... (tüm CSS kodunu buraya ekleyeceğiz - kısalık için kestim)
    
    return css;
}

String FlexKeyWeb::generateLoginHTML() {
    LanguageTexts texts = languageManager->getTexts();
    String error = server->hasArg("error") ? "1" : "0";
    
    String html = "<!DOCTYPE html><html><head><meta charset='UTF-8'><meta name='viewport' content='width=device-width, initial-scale=1.0, user-scalable=no'><title data-translate='systemAccess'>" + texts.systemAccess + "</title><link rel='stylesheet' href='/style.css'></head><body>";
    html += "<div class='container'>";
    html += "<div class='login-box'>";
    html += "<div class='header' style='text-align:center;margin-bottom:24px;'>";
    html += "<h1 style='font-size:2.5em;margin-bottom:0;color:#00ff88;'>FlexKey</h1>";
    html += "<div style='font-size:1.1em;opacity:0.7;color:#e5e7eb;margin-top:-4px;'><a href='https://www.smartkraft.ch' target='_blank' class='smartkraft' style='text-decoration:none;color:#e5e7eb;'>SmartKraft</a></div>";
    html += "<div class='language-selector' style='margin-top:16px;'>";
    html += "<button class='lang-btn"; if (languageManager->getCurrentLanguage() == "en") html += " active"; html += "' onclick='changeLanguage(\"en\")'>EN</button>";
    html += "<button class='lang-btn"; if (languageManager->getCurrentLanguage() == "de") html += " active"; html += "' onclick='changeLanguage(\"de\")'>DE</button>";
    html += "<button class='lang-btn"; if (languageManager->getCurrentLanguage() == "tr") html += " active"; html += "' onclick='changeLanguage(\"tr\")'>TR</button>";
    html += "</div>";
    html += "</div>";
    html += "<div class='login-form'>";
    html += "<h2 data-translate='systemAccess'>" + texts.systemAccess + "</h2>";
    if (error == "1") {
        html += "<div style='color:#ef4444;margin-bottom:16px;text-align:center;'>Invalid password</div>";
    }
    html += "<form method='POST' action='/login'>";
    html += "<div class='form-group'>";
    html += "<label data-translate='browserPassword'>" + texts.browserPassword + "</label>";
    html += "<input type='password' name='password' required placeholder='****' style='width:100%;'>";
    html += "</div>";
    html += "<button type='submit' class='btn-primary' style='width:100%;' data-translate='enterSystem'>" + texts.enterSystem + "</button>";
    html += "</form>";
    html += "</div>";
    html += "</div>";
    html += "<script>";
    html += "function changeLanguage(lang) {";
    html += "console.log('changeLanguage called with:', lang);";
    html += "fetch('/set-language', {";
    html += "method: 'POST',";
    html += "headers: {'Content-Type': 'application/x-www-form-urlencoded'},";
    html += "body: 'language=' + lang";
    html += "}).then(response => {";
    html += "if(response.ok) {";
    html += "return fetch('/get-translations');";
    html += "}";
    html += "throw new Error('Language change failed');";
    html += "}).then(response => response.json()).then(translations => {";
    html += "updateTranslations(translations);";
    html += "updateLanguageButtons(lang);";
    html += "}).catch(error => {";
    html += "console.error('Language change failed:', error);";
    html += "});";
    html += "}";
    html += "function updateTranslations(t) {";
    html += "document.querySelectorAll('[data-translate]').forEach(element => {";
    html += "const key = element.getAttribute('data-translate');";
    html += "if (t[key]) {";
    html += "if (element.tagName === 'TITLE') {";
    html += "element.textContent = t[key];";
    html += "} else {";
    html += "element.textContent = t[key];";
    html += "}";
    html += "}";
    html += "});";
    html += "}";
    html += "function updateLanguageButtons(activeLang) {";
    html += "document.querySelectorAll('.lang-btn').forEach(btn => btn.classList.remove('active'));";
    html += "var selector = '.lang-btn[onclick*=\\\"' + activeLang + '\\\"]';";
    html += "var el = document.querySelector(selector); if(el){ el.classList.add('active'); }";
    html += "}";
    html += "</script>";
    html += "</div></body></html>";
    
    return html;
}

String FlexKeyWeb::generateSetupHTML() {
    LanguageTexts texts = languageManager->getTexts();
    return "<html><head><title>" + texts.systemSettings + "</title></head><body><h1>" + texts.systemSettings + "</h1></body></html>";
}

String FlexKeyWeb::generateMainMenuHTML() {
    LanguageTexts texts = languageManager->getTexts();
    return "<html><head><title>" + texts.mainMenu + "</title></head><body><h1>" + texts.mainMenu + "</h1></body></html>";
}

// Placeholder implementations for other handlers
void FlexKeyWeb::handleDeviceSettings() {
    server->send(200, "text/html", "<html><body><h1>Device Settings</h1></body></html>");
}

void FlexKeyWeb::handleInfo() {
    server->send(200, "text/html", "<html><body><h1>Device Info</h1></body></html>");
}

void FlexKeyWeb::handleDeviceSettingsFragment() {
    server->send(200, "text/html", "<div>Device Settings Fragment</div>");
}

void FlexKeyWeb::handleInfoFragment() {
    server->send(200, "text/html", "<div>Info Fragment</div>");
}

void FlexKeyWeb::handleAuth() {
    server->send(200, "application/json", "{\"authenticated\":" + String(checkAuth() ? "true" : "false") + "}");
}

void FlexKeyWeb::handleRestart() {
    if (checkAuth()) {
        server->send(200, "text/plain", "Restarting...");
        if (flexkey) flexkey->restart();
    } else {
        server->send(401, "text/plain", "Unauthorized");
    }
}

void FlexKeyWeb::handleFactoryReset() {
    if (checkAuth()) {
        server->send(200, "text/plain", "Factory reset initiated...");
        if (flexkey) flexkey->factoryReset();
    } else {
        server->send(401, "text/plain", "Unauthorized");
    }
}

void FlexKeyWeb::handleLanguageChange() {
    if (server->hasArg("language")) {
        String lang = server->arg("language");
        if (languageManager) {
            languageManager->setLanguage(lang);
        }
        server->send(200, "text/plain", "Language changed");
    } else {
        server->send(400, "text/plain", "No language specified");
    }
}

void FlexKeyWeb::handleGetTranslations() {
    if (languageManager) {
        String jsonStr = languageManager->getTranslationsJSON();
        server->send(200, "application/json", jsonStr);
    } else {
        server->send(500, "text/plain", "Language manager not available");
    }
}

void FlexKeyWeb::handlePasswordUpdate() {
    server->send(200, "text/plain", "Password update not implemented");
}

void FlexKeyWeb::handleWifiScan() {
    server->send(200, "application/json", "[]"); // Empty networks list for now
}

void FlexKeyWeb::handleWifiUpdate() {
    server->send(200, "text/plain", "WiFi update not implemented");
}

void FlexKeyWeb::handleToggleAP() {
    server->send(200, "text/plain", "AP toggle not implemented");
}

// FlexKey_RF helper delegations  
String FlexKeyWeb::fx2NormalizeUid(const String &raw) {
    return flexkey ? flexkey->fx2NormalizeUid(raw) : "";
}

bool FlexKeyWeb::fx2LoadConfig(DynamicJsonDocument &doc) {
    return flexkey ? flexkey->fx2LoadConfig(doc) : false;
}

bool FlexKeyWeb::fx2SaveConfig(DynamicJsonDocument &doc) {
    return flexkey ? flexkey->fx2SaveConfig(doc) : false;
}

void FlexKeyWeb::fx2ComputeStats(const DynamicJsonDocument &doc, int &gCount, int &uCount, int &urlCount) {
    if (flexkey) {
        flexkey->fx2ComputeStats(doc, gCount, uCount, urlCount);
    }
}
