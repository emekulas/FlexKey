#include "FlexKey.h"
#include "FlexKey_RF.h"
#include "FlexKey_web.h"
#include <SPIFFS.h>
#include <ArduinoJson.h>
#include <mbedtls/sha256.h>

#define FACTORY_RESET_ONCE_FLAG "/system/factory_reset_once.flag"

// SHA-256 hash fonksiyonu (hex string döndürür)
String sha256(const String &input) {
    byte hash[32];
    mbedtls_sha256_context ctx;
    mbedtls_sha256_init(&ctx);
    mbedtls_sha256_starts(&ctx, 0);
    mbedtls_sha256_update(&ctx, (const unsigned char*)input.c_str(), input.length());
    mbedtls_sha256_finish(&ctx, hash);
    mbedtls_sha256_free(&ctx);
    String hashStr = "";
    for (int i = 0; i < 32; i++) {
        if (hash[i] < 16) hashStr += "0";
        hashStr += String(hash[i], HEX);
    }
    hashStr.toLowerCase();
    return hashStr;
}

FlexKey::FlexKey() : server(80) {
    isAuthenticated = false;
    isFirstSetup = false;
    currentLanguage = "en";
    apEnabled = true; // Varsayılan olarak AP açık
    languageManager.setLanguage(currentLanguage);
    webUI = nullptr; // Initialize web UI pointer
}

void FlexKey::begin() {
    Serial.begin(115200);
    Serial.println("FlexKey System Starting...");

    // Initialize hardware
    initializePins();

    // Initialize file system
    initializeFileSystem();

    // Generate chip ID and AP SSID
    generateChipID();

    // Print device ID to serial
    Serial.print("Device ID: ");
    Serial.println(chipID);

    // Check if this is first setup
    isFirstSetup = isFirstTimeSetup();

    // Create system configuration
    createSystemConfig();

    // AP modunu configten oku (ilk kurulumda true olur)
    if (SPIFFS.exists(SYSTEM_CONFIG_PATH)) {
        File configFile = SPIFFS.open(SYSTEM_CONFIG_PATH, "r");
        if (configFile) {
            DynamicJsonDocument doc(1024);
            DeserializationError error = deserializeJson(doc, configFile);
            configFile.close();
            if (!error && doc["apEnabled"].is<bool>()) {
                apEnabled = doc["apEnabled"].as<bool>();
            }
        }
    }

    // --- WiFi Ayarları ---
    bool wifiConnected = false;
    bool useStaticIP = false;
    String wifiSSID = "";
    String wifiPassword = "";
    IPAddress staticIP, gateway, subnet;

    if (SPIFFS.exists("/user/wifi.json")) {
        File wifiFile = SPIFFS.open("/user/wifi.json", "r");
        if (wifiFile) {
            DynamicJsonDocument wifiDoc(512);
            DeserializationError error = deserializeJson(wifiDoc, wifiFile);
            wifiFile.close();
            if (!error) {
                wifiSSID = wifiDoc["ssid"].as<String>();
                wifiPassword = wifiDoc["password"].as<String>();
                useStaticIP = wifiDoc["useStaticIP"].as<bool>();
                if (useStaticIP) {
                    staticIP.fromString(wifiDoc["staticIP"].as<String>());
                    gateway.fromString(wifiDoc["gateway"].as<String>());
                    subnet.fromString(wifiDoc["subnet"].as<String>());
                }
            }
        }
    }

    // AP/STA modunu ayarla
    setAPMode(apEnabled);
    if (apEnabled) {
        Serial.println("AP Mode Started");
        Serial.println("SSID: " + apSSID);
        Serial.println("IP: " + WiFi.softAPIP().toString());
    } else {
        Serial.println("AP Mode Disabled");
    }
    Serial.println("Browser Password: " + browserPassword);

    // Eğer WiFi ayarları varsa STA modunda bağlanmayı dene
    if (wifiSSID.length() > 0) {
        if (useStaticIP && staticIP && gateway && subnet) {
            WiFi.config(staticIP, gateway, subnet);
        }
        WiFi.begin(wifiSSID.c_str(), wifiPassword.c_str());
        Serial.print("Connecting to WiFi: ");
        Serial.println(wifiSSID);
        unsigned long startAttemptTime = millis();
        while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < 10000) {
            delay(500);
            Serial.print(".");
        }
        if (WiFi.status() == WL_CONNECTED) {
            wifiConnected = true;
            Serial.println("");
            Serial.println("WiFi Connected!");
            Serial.print("IP Address: ");
            Serial.println(WiFi.localIP());
        } else {
            Serial.println("");
            Serial.println("WiFi Connection Failed");
        }
    }

    // Initialize web UI module
    if (!webUI) {
        webUI = new FlexKeyWeb(this, &server, &languageManager);
        webUI->begin();
    }

    // Setup web server
    setupWebServer();
    server.begin();

    Serial.println("Web Server Started on port 80");
    Serial.println("FlexKey System Ready!");
}

void FlexKey::loop() {
    server.handleClient();
    handleButtonPress();
}

void FlexKey::initializePins() {
    pinMode(BUTTON_PIN, INPUT_PULLUP);
    pinMode(RELAY_PIN, OUTPUT);
    digitalWrite(RELAY_PIN, LOW);
}

void FlexKey::initializeFileSystem() {
    if (!SPIFFS.begin(true)) {
        Serial.println("SPIFFS Mount Failed");
        return;
    }
    
    // Create system directories if they don't exist
    if (!SPIFFS.exists("/system")) {
        Serial.println("Creating system directory structure...");
    }
}

void FlexKey::generateChipID() {
    uint64_t chipid = ESP.getEfuseMac();
    chipID = String((uint32_t)(chipid >> 32), HEX) + String((uint32_t)chipid, HEX);
    chipID.toUpperCase();
    // Use only last 6 characters for AP SSID
    String shortID = chipID.substring(chipID.length() - 6);
    apSSID = "FlexKey_" + shortID;
}

void FlexKey::createSystemConfig() {
    // Try to load existing configuration first
    if (SPIFFS.exists(SYSTEM_CONFIG_PATH)) {
        File configFile = SPIFFS.open(SYSTEM_CONFIG_PATH, "r");
        if (configFile) {
            DynamicJsonDocument doc(1024);
            DeserializationError error = deserializeJson(doc, configFile);
            configFile.close();
            if (!error) {
                // Hashli şifreyi yükle
                if (doc["browserPassword"]) {
                    browserPassword = doc["browserPassword"].as<String>();
                    Serial.println("Loaded existing browser password hash");
                }
                if (doc["language"]) {
                    currentLanguage = doc["language"].as<String>();
                    languageManager.setLanguage(currentLanguage);
                }
                if (doc["apEnabled"].is<bool>()) {
                    apEnabled = doc["apEnabled"].as<bool>();
                }
                return;
            }
        }
    }
    // Eğer config yoksa, ilk kurulumda kullanıcıdan şifre alınacak. Otomatik şifre atama yok!
}

void FlexKey::generateBrowserPassword() {
    // 8 karakterlik random şifre üret, hash'le ve sakla
    String rawPass = "";
    for (int i = 0; i < 8; i++) {
        rawPass += chipID.charAt(i % chipID.length());
    }
    rawPass.toUpperCase();
    browserPassword = sha256(rawPass); // Sadece hash saklanır
    Serial.println("Default browser password (hash): " + browserPassword);
}

// Dil yönetimini languageManager'a devredelim
void FlexKey::setLanguage(String lang) {
    if (lang == "en" || lang == "de" || lang == "tr") {
        currentLanguage = lang;
        languageManager.setLanguage(lang);
        
        // Konfigürasyonu kaydet
        DynamicJsonDocument doc(1024);
        if (SPIFFS.exists(SYSTEM_CONFIG_PATH)) {
            File configFile = SPIFFS.open(SYSTEM_CONFIG_PATH, "r");
            if (configFile) {
                deserializeJson(doc, configFile);
                configFile.close();
            }
        }
        doc["language"] = currentLanguage;
        File configFile = SPIFFS.open(SYSTEM_CONFIG_PATH, "w");
        if (configFile) {
            serializeJson(doc, configFile);
            configFile.close();
        }
    }
}

bool FlexKey::isFirstTimeSetup() {
    return !SPIFFS.exists(FIRST_SETUP_FLAG);
}

void FlexKey::markSetupComplete() {
    File file = SPIFFS.open(FIRST_SETUP_FLAG, "w");
    if (file) {
        file.println("setup_complete");
        file.close();
    }
    // AP modunu configte açık bırak (ilk kurulumdan sonra)
    apEnabled = true;
    // Sistem config dosyasına da yaz
    DynamicJsonDocument doc(1024);
    doc["chipID"] = chipID;
    doc["apSSID"] = apSSID;
    doc["browserPassword"] = browserPassword;
    doc["language"] = currentLanguage;
    doc["version"] = "1.0.0";
    doc["systemReady"] = true;
    doc["apEnabled"] = apEnabled;
    File configFile = SPIFFS.open(SYSTEM_CONFIG_PATH, "w");
    if (configFile) {
        serializeJson(doc, configFile);
        configFile.close();
    }
}
// AP modunu aç/kapat fonksiyonu
void FlexKey::setAPMode(bool enable) {
    if (enable) {
        WiFi.mode(WIFI_AP_STA);
        IPAddress ap_local_IP(192, 168, 111, 1);
        IPAddress ap_gateway(192, 168, 111, 1);
        IPAddress ap_subnet(255, 255, 255, 0);
        WiFi.softAPConfig(ap_local_IP, ap_gateway, ap_subnet);
        WiFi.softAP(apSSID.c_str());
    } else {
        WiFi.softAPdisconnect(true);
        WiFi.mode(WIFI_STA);
    }
}

void FlexKey::setupWebServer() {
    // Delegate all web routing to the webUI module
    if (webUI) {
        webUI->setupWebRoutes();
    }
}
    // FlexKey_2 UI fragment (hafif, gerçek verilerle)
    server.on("/fxrf/ui", HTTP_GET, [this]() {
        if (!checkAuth()) { server.send(401, "text/plain", "Unauthorized"); return; }
        
        // Çeviri metinlerini al
        LanguageTexts texts = languageManager.getTexts();
        
        DynamicJsonDocument cfg(16384); fxrfLoadConfig(cfg);
        int totalGroups=0,totalUIDs=0,totalURLs=0; fxrfComputeStats(cfg,totalGroups,totalUIDs,totalURLs);
        String relayMode = cfg["relayMode"].is<const char*>() ? String(cfg["relayMode"].as<const char*>()) : String("NO");

    String html; html.reserve(12288);
        html += "<div class='device-settings-main compact-content' style='max-width:720px;width:100%;margin-left:auto;margin-right:auto;'>";
        html += "<div class='expandable-section'>";
        html += "<div class='section-header'><span>" + texts.flexkeySettings + "</span></div>";
        html += "<div class='section-content' style='display:block;'>";
        html += "<div class='info-grid'>";
            html += "<div class='info-item'><label>" + texts.groups + "</label><span>" + String(totalGroups) + "</span></div>";
            html += "<div class='info-item'><label>UID</label><span>" + String(totalUIDs) + "</span></div>";
            html += "<div class='info-item'><label>URL</label><span>" + String(totalURLs) + "</span></div>";
            html += "</div>";
        html += "</div></div>";
        
        // Röle Ayarları Bölümü
        html += "<div class='expandable-section'>";
        html += "<div class='section-header'><span>" + texts.relayDefaultSettings + "</span></div>";
        html += "<div class='section-content' style='display:block;'>";
        html += "<div style='background:rgba(26,26,26,0.6);padding:12px;border-radius:8px;margin-bottom:15px;'>";
        html += "<h4 style='margin:0 0 8px 0;color:#e5e7eb;font-size:0.95em;'>Röle Tipi Açıklaması:</h4>";
        html += "<div style='font-size:0.85em;color:#9ca3af;line-height:1.4;'>";
        html += "<p style='margin:4px 0;'><strong>NO (Normally Open):</strong> Röle normalde AÇIK. Enerji verildiğinde KAPANIR.</p>";
        html += "<p style='margin:4px 0;'><strong>NC (Normally Closed):</strong> Röle normalde KAPALI. Enerji verildiğinde AÇILIR.</p>";
        html += "</div></div>";
        
        html += "<div style='display:flex;gap:12px;align-items:center;flex-wrap:wrap;'>";
        html += "<div class='form-group' style='margin-bottom:10px;'>";
        html += "<label style='margin-right:10px;'>Röle Modu:</label>";
        html += "<div class='toggle-container' style='display:inline-flex;background:rgba(255,255,255,0.1);border-radius:20px;padding:2px;'>";
        html += "<button id='fxrf-toggle-no' class='toggle-btn " + String(relayMode == "NO" ? "active" : "") + "' onclick='fxrfToggleRelay(\"NO\")' style='padding:6px 12px;border-radius:18px;border:none;background:" + String(relayMode == "NO" ? "#10b981" : "transparent") + ";color:#fff;cursor:pointer;transition:all 0.3s;'>NO</button>";
        html += "<button id='fxrf-toggle-nc' class='toggle-btn " + String(relayMode == "NC" ? "active" : "") + "' onclick='fxrfToggleRelay(\"NC\")' style='padding:6px 12px;border-radius:18px;border:none;background:" + String(relayMode == "NC" ? "#10b981" : "transparent") + ";color:#fff;cursor:pointer;transition:all 0.3s;'>NC</button>";
        html += "</div>";
        html += "</div>";
        html += "</div>";
        html += "</div></div>";
        
        // Yeni Grup Oluşturma Bölümü
        html += "<div class='expandable-section'>";
        html += "<div class='section-header'><span>" + texts.addNewGroup + "</span></div>";
        html += "<div class='section-content' style='display:block;'>";
        html += "<div style='display:flex;gap:12px;align-items:center;flex-wrap:wrap;'>";
        html += "<input id='fxrf-new-group-name' type='text' placeholder='" + texts.groupName + "...' style='flex:1;min-width:200px;padding:10px 14px;border-radius:8px;background:rgba(26,26,26,0.8);color:#e5e7eb;border:2px solid rgba(255,255,255,0.1);'>";
        html += "<button id='fxrf-add-group' class='btn-primary'>" + texts.add + "</button>";
        html += "</div>";
        html += "</div></div>";
        
        // Gruplar Listesi
        html += "<div class='expandable-section'>";
        html += "<div class='section-header'><span>" + texts.groups + "</span></div>";
        html += "<div class='section-content' style='display:block;'>";
        html += "<div id='fxrf-groups-container' class='list-container' style='margin-top:12px;'></div>";
        html += "</div></div>";
        html += "</div>";

    // Basit JS renderer
    html += "<script>\n";
    html += "function fxrfEncode(x){return encodeURIComponent(x);}\n";
    html += "function fxrfRenderGroups(groups){\n";
    html += "  const cont=document.getElementById('fxrf-groups-container');\n";
    html += "  if(!cont) return;\n";
    html += "  let html='';\n";
    html += "  groups.forEach((g,gi)=>{\n";
    html += "    const uidCount=(g.uids||[]).length; const urlCount=(g.urls||[]).length;\n";
    html += "    html+=`<div class=\\\"expandable-section\\\">`+\n";
    html += "      `<div class=\\\"section-header\\\"><span>${g.name||('Grup '+(gi+1))}</span>`+\n";
    html += "      `<div style=\\\"margin-left:auto;display:flex;gap:8px;align-items:center;\\\">`+\n";
    html += "      `<span style=\\\"opacity:.7\\\">UID:${uidCount} URL:${urlCount}</span>`+\n";
    html += "      `<button class=\\\"btn-secondary\\\" onclick=\\\"fxrfDeleteGroup(${gi})\\\">" + texts.delete_ + "</button></div></div>`+\n";
    html += "      `<div class=\\\"section-content\\\" style=\\\"display:block;\\\">`+\n";
    html += "        `<div style=\\\"display:flex;gap:10px;align-items:center;flex-wrap:wrap;\\\">`+\n";
    html += "        `<input id=\\\"fxrf-uid-${gi}\\\" type=\\\"text\\\" placeholder=\\\"UID (AA:BB:CC...)\\\" style=\\\"min-width:160px;\\\">`+\n";
    html += "        `<button class=\\\"btn-primary\\\" onclick=\\\"fxrfAddUid(${gi})\\\">" + texts.add + " UID</button>`+\n";
    html += "        `<input id=\\\"fxrf-url-${gi}\\\" type=\\\"text\\\" placeholder=\\\"URL (http...)\\\" style=\\\"flex:1;min-width:200px;\\\">`+\n";
    html += "        `<button class=\\\"btn-primary\\\" onclick=\\\"fxrfAddUrl(${gi})\\\">" + texts.add + " URL</button>`+\n";
    html += "        `</div>`+\n";
    html += "        `<div style=\\\"margin-top:10px;display:grid;grid-template-columns:1fr auto;gap:6px;\\\">`+\n";
    html += "        `${(g.uids||[]).map((u,i)=>`<div class=\\\"pill\\\">${u}</div><button class=\\\"btn-secondary\\\" onclick=\\\"fxrfDelUid(${gi},${i})\\\">" + texts.delete_ + "</button>`).join('')}`+\n";
    html += "        `</div>`+\n";
    html += "        `<div style=\\\"margin-top:10px;display:grid;grid-template-columns:1fr auto;gap:6px;\\\">`+\n";
    html += "        `${(g.urls||[]).map((u,i)=>`<div class=\\\"pill\\\" style=\\\"overflow:hidden;text-overflow:ellipsis;white-space:nowrap;\\\">${u}</div><button class=\\\"btn-secondary\\\" onclick=\\\"fxrfDelUrl(${gi},${i})\\\">" + texts.delete_ + "</button>`).join('')}`+\n";
    html += "        `</div>`+\n";
    html += "      `</div>`+\n";
    html += "    `</div>`;\n";
    html += "  });\n";
    html += "  cont.innerHTML=html;\n";
    html += "}\n";
    html += "function fxrfLoadAndRender(){fetch('/fxrf/groups').then(r=>{if(r.ok) return r.json(); throw new Error('HTTP '+r.status);}).then(gs=>{fxrfRenderGroups(gs);}).catch(err=>{console.error('Grup yükleme hatası:', err);});}\n";
    html += "function fxrfDeleteGroup(i){if(confirm('Bu grubu silmek istediğinizden emin misiniz?')) fetch('/fxrf/groups?index='+i,{method:'DELETE'}).then(response=>{if(response.ok) fxrfLoadAndRender(); else alert('Grup silinemedi: '+response.status);}).catch(err=>{alert('Ağ hatası: '+err.message);});}\n";
    html += "function fxrfAddUid(i){const el=document.getElementById('fxrf-uid-'+i); if(!el) return; const v=el.value.trim(); if(!v) return; fetch('/fxrf/uids',{method:'POST',headers:{'Content-Type':'application/x-www-form-urlencoded'},body:'group='+i+'&uid='+fxrfEncode(v)}).then(()=>{el.value=''; fxrfLoadAndRender();});}\n";
    html += "function fxrfDelUid(i,j){fetch('/fxrf/uids?group='+i+'&index='+j,{method:'DELETE'}).then(()=>fxrfLoadAndRender());}\n";
    html += "function fxrfAddUrl(i){const el=document.getElementById('fxrf-url-'+i); if(!el) return; const v=el.value.trim(); if(!v) return; if(v.length>1000) {alert('URL too long'); return;} fetch('/fxrf/urls',{method:'POST',headers:{'Content-Type':'application/x-www-form-urlencoded'},body:'group='+i+'&url='+fxrfEncode(v)}).then(()=>{el.value=''; fxrfLoadAndRender();});}\n";
    html += "function fxrfDelUrl(i,j){fetch('/fxrf/urls?group='+i+'&index='+j,{method:'DELETE'}).then(()=>fxrfLoadAndRender());}\n";
    html += "document.getElementById('fxrf-add-group').addEventListener('click',function(e){e.preventDefault(); console.log('Grup ekleme butonu tıklandı'); const el=document.getElementById('fxrf-new-group-name'); const name=(el&&el.value?el.value.trim():''); console.log('Grup adı:', name); if(!name) {alert('Grup adı gerekli!'); return;} console.log('POST isteği gönderiliyor...'); fetch('/fxrf/groups',{method:'POST',headers:{'Content-Type':'application/x-www-form-urlencoded'},body:'name='+fxrfEncode(name)}).then(response=>{console.log('Response alındı:', response.status); if(response.ok) {el.value=''; fxrfLoadAndRender();} else {alert('Grup oluşturulamadı: '+response.status);}}).catch(err=>{console.error('Fetch hatası:', err); alert('Ağ hatası: '+err.message);});});\n";
    html += "function fxrfToggleRelay(mode){const noBtn=document.getElementById('fxrf-toggle-no'); const ncBtn=document.getElementById('fxrf-toggle-nc'); noBtn.style.background=mode==='NO'?'#10b981':'transparent'; ncBtn.style.background=mode==='NC'?'#10b981':'transparent'; fetch('/fxrf/relay-mode',{method:'POST',headers:{'Content-Type':'application/x-www-form-urlencoded'},body:'mode='+fxrfEncode(mode)}).then(()=>{console.log('Relay mode updated to:', mode);}).catch(err=>{console.error('Relay update error:', err);});}\n";
    html += "fxrfLoadAndRender();\n";
    html += "document.addEventListener('DOMContentLoaded', function() {\n";
    html += "  console.log('DOM yüklendi');\n";
    html += "  const addBtn = document.getElementById('fxrf-add-group');\n";
    html += "  const nameInput = document.getElementById('fxrf-new-group-name');\n";
    html += "  console.log('Add button:', addBtn);\n";
    html += "  console.log('Name input:', nameInput);\n";
    html += "});\n";
    html += "</script>";
        server.send(200, "text/html", html);
    });

    // FXRF API
    server.on("/fxrf/stats", HTTP_GET, [this]() {
        if (!checkAuth()) { server.send(401, "application/json", "{\"error\":\"unauthorized\"}"); return; }
        DynamicJsonDocument doc(16384); fxrfLoadConfig(doc);
        int g=0,u=0,ur=0; fxrfComputeStats(doc,g,u,ur);
        DynamicJsonDocument out(256);
        out["groups"] = g; out["uids"] = u; out["urls"] = ur;
        String s; serializeJson(out, s); server.send(200, "application/json", s);
    });

    server.on("/fxrf/groups", HTTP_GET, [this]() {
        if (!checkAuth()) { server.send(401, "application/json", "{\"error\":\"unauthorized\"}"); return; }
        DynamicJsonDocument doc(16384); fxrfLoadConfig(doc);
        String s; serializeJson(doc["groups"], s); server.send(200, "application/json", s);
    });

    server.on("/fxrf/groups", HTTP_POST, [this]() {
        if (!checkAuth()) { server.send(401, "application/json", "{\"error\":\"unauthorized\"}"); return; }
        DynamicJsonDocument doc(32768); fxrfLoadConfig(doc);
        JsonArray groups = doc["groups"].as<JsonArray>();
        if (groups.size() >= 10) { server.send(400, "application/json", "{\"error\":\"max_groups\"}"); return; }
        String name = server.hasArg("name") ? server.arg("name") : String("");
        if (name.length() == 0 || name.length() > 40) { server.send(400, "application/json", "{\"error\":\"invalid_name\"}"); return; }
    JsonObject g = groups.createNestedObject();
    g["name"] = name;
    g.createNestedArray("uids");
    g.createNestedArray("urls");
        JsonObject relay = g.createNestedObject("relay");
        relay["contact"] = "NO"; relay["mode"] = "pulse"; relay["durationMs"] = 500; relay["delayMs"] = 0; // defaults
        fxrfSaveConfig(doc);
        server.send(200, "application/json", "{\"status\":\"ok\"}");
    });

    server.on("/fxrf/groups", HTTP_DELETE, [this]() {
        if (!checkAuth()) { server.send(401, "application/json", "{\"error\":\"unauthorized\"}"); return; }
        if (!server.hasArg("index")) { server.send(400, "application/json", "{\"error\":\"missing_index\"}"); return; }
        int idx = server.arg("index").toInt();
        DynamicJsonDocument doc(32768); fxrfLoadConfig(doc);
        JsonArray groups = doc["groups"].as<JsonArray>();
        if (idx < 0 || idx >= (int)groups.size()) { server.send(400, "application/json", "{\"error\":\"invalid_index\"}"); return; }
        groups.remove(idx); fxrfSaveConfig(doc);
        server.send(200, "application/json", "{\"status\":\"ok\"}");
    });

    server.on("/fxrf/uids", HTTP_POST, [this]() {
        if (!checkAuth()) { server.send(401, "application/json", "{\"error\":\"unauthorized\"}"); return; }
        if (!server.hasArg("group") || !server.hasArg("uid")) { server.send(400, "application/json", "{\"error\":\"missing_params\"}"); return; }
        int gidx = server.arg("group").toInt();
        String norm = fxrfNormalizeUid(server.arg("uid"));
        if (norm.length() == 0) { server.send(400, "application/json", "{\"error\":\"invalid_uid\"}"); return; }
        DynamicJsonDocument doc(32768); fxrfLoadConfig(doc);
        JsonArray groups = doc["groups"].as<JsonArray>();
        if (gidx < 0 || gidx >= (int)groups.size()) { server.send(400, "application/json", "{\"error\":\"invalid_group\"}"); return; }
        // UID tekil ve tek grup kuralı
        for (JsonObject g : groups) {
            for (JsonVariant v : g["uids"].as<JsonArray>()) {
                if (v.as<String>() == norm) { server.send(400, "application/json", "{\"error\":\"uid_exists\"}"); return; }
            }
        }
        JsonArray uids = groups[gidx]["uids"].as<JsonArray>();
        if (uids.size() >= 32) { server.send(400, "application/json", "{\"error\":\"max_uids\"}"); return; }
        uids.add(norm); fxrfSaveConfig(doc);
        server.send(200, "application/json", "{\"status\":\"ok\"}");
    });

    server.on("/fxrf/uids", HTTP_DELETE, [this]() {
        if (!checkAuth()) { server.send(401, "application/json", "{\"error\":\"unauthorized\"}"); return; }
        if (!server.hasArg("group") || !server.hasArg("index")) { server.send(400, "application/json", "{\"error\":\"missing_params\"}"); return; }
        int gidx = server.arg("group").toInt(); int idx = server.arg("index").toInt();
        DynamicJsonDocument doc(32768); fxrfLoadConfig(doc);
        JsonArray groups = doc["groups"].as<JsonArray>();
        if (gidx < 0 || gidx >= (int)groups.size()) { server.send(400, "application/json", "{\"error\":\"invalid_group\"}"); return; }
        JsonArray uids = groups[gidx]["uids"].as<JsonArray>();
        if (idx < 0 || idx >= (int)uids.size()) { server.send(400, "application/json", "{\"error\":\"invalid_index\"}"); return; }
        uids.remove(idx); fxrfSaveConfig(doc); server.send(200, "application/json", "{\"status\":\"ok\"}");
    });

    server.on("/fxrf/urls", HTTP_POST, [this]() {
        if (!checkAuth()) { server.send(401, "application/json", "{\"error\":\"unauthorized\"}"); return; }
        if (!server.hasArg("group") || !server.hasArg("url")) { server.send(400, "application/json", "{\"error\":\"missing_params\"}"); return; }
        int gidx = server.arg("group").toInt(); String url = server.arg("url");
        if (url.length() == 0 || url.length() > 1000) { server.send(400, "application/json", "{\"error\":\"invalid_url\"}"); return; }
        DynamicJsonDocument doc(32768); fxrfLoadConfig(doc);
        JsonArray groups = doc["groups"].as<JsonArray>();
        if (gidx < 0 || gidx >= (int)groups.size()) { server.send(400, "application/json", "{\"error\":\"invalid_group\"}"); return; }
        JsonArray urls = groups[gidx]["urls"].as<JsonArray>();
        if (urls.size() >= 16) { server.send(400, "application/json", "{\"error\":\"max_urls\"}"); return; }
        urls.add(url); fxrfSaveConfig(doc); server.send(200, "application/json", "{\"status\":\"ok\"}");
    });

    server.on("/fxrf/urls", HTTP_DELETE, [this]() {
        if (!checkAuth()) { server.send(401, "application/json", "{\"error\":\"unauthorized\"}"); return; }
        if (!server.hasArg("group") || !server.hasArg("index")) { server.send(400, "application/json", "{\"error\":\"missing_params\"}"); return; }
        int gidx = server.arg("group").toInt(); int idx = server.arg("index").toInt();
        DynamicJsonDocument doc(32768); fxrfLoadConfig(doc);
        JsonArray groups = doc["groups"].as<JsonArray>();
        if (gidx < 0 || gidx >= (int)groups.size()) { server.send(400, "application/json", "{\"error\":\"invalid_group\"}"); return; }
        JsonArray urls = groups[gidx]["urls"].as<JsonArray>();
        if (idx < 0 || idx >= (int)urls.size()) { server.send(400, "application/json", "{\"error\":\"invalid_index\"}"); return; }
        urls.remove(idx); fxrfSaveConfig(doc); server.send(200, "application/json", "{\"status\":\"ok\"}");
    });

    server.on("/fxrf/relay-mode", HTTP_POST, [this]() {
        if (!checkAuth()) { server.send(401, "application/json", "{\"error\":\"unauthorized\"}"); return; }
        if (!server.hasArg("mode")) { server.send(400, "application/json", "{\"error\":\"missing_mode\"}"); return; }
        String mode = server.arg("mode");
        if (!((mode == "NO") || (mode == "NC"))) { server.send(400, "application/json", "{\"error\":\"invalid_mode\"}"); return; }
        DynamicJsonDocument doc(8192); fxrfLoadConfig(doc);
        doc["relayMode"] = mode; fxrfSaveConfig(doc);
        server.send(200, "application/json", "{\"status\":\"ok\"}");
    });

    server.on("/fxrf/relay-mode", HTTP_GET, [this]() {
        if (!checkAuth()) { server.send(401, "application/json", "{\"error\":\"unauthorized\"}"); return; }
        DynamicJsonDocument doc(8192); fxrfLoadConfig(doc);
        DynamicJsonDocument out(64);
        out["mode"] = doc["relayMode"].is<const char*>() ? doc["relayMode"].as<const char*>() : "NO";
        String s; serializeJson(out, s); server.send(200, "application/json", s);
    });
}

void FlexKey::handleCompleteSetup() {
    if (server.hasArg("plain")) {
        DynamicJsonDocument doc(1024);
        deserializeJson(doc, server.arg("plain"));

        // Şifreyi hashle ve config dosyasına kaydet
        String newPassword = doc["browserPassword"].as<String>();
        if (newPassword.length() < 4) {
            server.send(400, "text/plain", "Password too short");
            return;
        }
        browserPassword = sha256(newPassword);
        // Sistem konfigürasyonunu güncelle
        DynamicJsonDocument sysDoc(1024);
        sysDoc["chipID"] = chipID;
        sysDoc["apSSID"] = apSSID;
        sysDoc["browserPassword"] = browserPassword;
        sysDoc["language"] = currentLanguage;
        sysDoc["version"] = "1.0.0";
        sysDoc["systemReady"] = true;
        File configFile = SPIFFS.open(SYSTEM_CONFIG_PATH, "w");
        if (configFile) {
            serializeJson(sysDoc, configFile);
            configFile.close();
        }

        // WiFi ayarlarını kaydet
        DynamicJsonDocument wifiDoc(512);
        wifiDoc["ssid"] = doc["wifiSSID"].as<String>();
        wifiDoc["password"] = doc["wifiPassword"].as<String>();
        wifiDoc["useStaticIP"] = doc["useStaticIP"].as<bool>();
        wifiDoc["staticIP"] = doc["staticIP"].as<String>();
        wifiDoc["gateway"] = doc["gateway"].as<String>();
        wifiDoc["subnet"] = doc["subnet"].as<String>();
        File wifiFile = SPIFFS.open("/user/wifi.json", "w");
        if (wifiFile) {
            serializeJson(wifiDoc, wifiFile);
            wifiFile.close();
        }

        markSetupComplete();
        isFirstSetup = false;
        isAuthenticated = true;

        server.send(200, "text/plain", "Setup completed");
        delay(10000); // 10 saniye bekle
        restart();
    } else {
        server.send(400, "text/plain", "Invalid data");
    }
}

// WiFi ayarlarını güncelleyen endpoint
void FlexKey::handleWifiUpdate() {
    if (!checkAuth()) {
        server.send(401, "text/plain", "Unauthorized");
        return;
    }
    if (server.hasArg("plain")) {
        DynamicJsonDocument doc(512);
        deserializeJson(doc, server.arg("plain"));
        DynamicJsonDocument wifiDoc(512);
        wifiDoc["ssid"] = doc["wifiSSID"].as<String>();
        wifiDoc["password"] = doc["wifiPassword"].as<String>();
        wifiDoc["useStaticIP"] = doc["useStaticIP"].as<bool>();
        wifiDoc["staticIP"] = doc["staticIP"].as<String>();
        wifiDoc["gateway"] = doc["gateway"].as<String>();
        wifiDoc["subnet"] = doc["subnet"].as<String>();
        File wifiFile = SPIFFS.open("/user/wifi.json", "w");
        if (wifiFile) {
            serializeJson(wifiDoc, wifiFile);
            wifiFile.close();
            server.send(200, "text/plain", "WiFi ayarları güncellendi. Yeniden başlatmanız gerekebilir.");
        } else {
            server.send(500, "text/plain", "WiFi ayarları kaydedilemedi");
        }
    } else {
        server.send(400, "text/plain", "Invalid data");
    }
}







void FlexKey::handleLanguageChange() {
    if (server.hasArg("language")) {
        String newLang = server.arg("language");
        if (newLang == "en" || newLang == "de" || newLang == "tr") {
            setLanguage(newLang);
            server.send(200, "text/plain", "Language changed");
        } else {
            server.send(400, "text/plain", "Invalid language");
        }
    } else {
        server.send(400, "text/plain", "Invalid language");
    }
}

void FlexKey::handleGetTranslations() {
    String json = languageManager.getTranslationJSON();
    server.send(200, "application/json", json);
}

void FlexKey::handlePasswordUpdate() {
    // İlk kurulum sırasında auth kontrolü yapma
    if (!isFirstSetup && !checkAuth()) {
        server.send(401, "text/plain", "Unauthorized");
        return;
    }
    if (server.hasArg("password")) {
        String newPassword = server.arg("password");
        if (newPassword.length() >= 4) {
            String newHash = sha256(newPassword);
            browserPassword = newHash;
            // Update the system config file
            DynamicJsonDocument doc(1024);
            doc["chipID"] = chipID;
            doc["apSSID"] = apSSID;
            doc["browserPassword"] = browserPassword;
            doc["language"] = currentLanguage;
            doc["version"] = "1.0.0";
            doc["systemReady"] = true;
            File configFile = SPIFFS.open(SYSTEM_CONFIG_PATH, "w");
            if (configFile) {
                serializeJson(doc, configFile);
                configFile.close();
                server.send(200, "text/plain", "success");
            } else {
                server.send(500, "text/plain", "Failed to save");
            }
        } else {
            server.send(400, "text/plain", "Password too short");
        }
    } else {
        server.send(400, "text/plain", "No password provided");
    }
}











void FlexKey::handleButtonPress() {
    bool currentButtonState = digitalRead(BUTTON_PIN) == LOW;
    
    if (currentButtonState && !buttonPressed) {
        buttonPressed = true;
        buttonPressTime = millis();
    } else if (!currentButtonState && buttonPressed) {
        buttonPressed = false;
        unsigned long pressDuration = millis() - buttonPressTime;
        
        if (pressDuration >= 10000) {
            Serial.println("Button Factory Reset triggered");
            factoryReset();
        } else if (pressDuration >= 1000) {
            Serial.println("Button Restart triggered");
            restart();
        }
    }
}

bool FlexKey::checkAuth() {
    return isAuthenticated;
}

void FlexKey::restart() {
    Serial.println("System restarting...");
    ESP.restart();
}

void FlexKey::factoryReset() {
    Serial.println("Factory reset initiated...");
    
    // Kullanıcı verilerini sil
    if (SPIFFS.exists("/user")) {
        File root = SPIFFS.open("/user");
        File file = root.openNextFile();
        while (file) {
            String fileName = file.name();
            file.close();
            SPIFFS.remove(fileName);
            file = root.openNextFile();
        }
        root.close();
    }
    // WiFi ayar dosyasını da ayrıca sil (garanti için)
    if (SPIFFS.exists("/user/wifi.json")) {
        SPIFFS.remove("/user/wifi.json");
    }
    // Sistem konfigürasyonunu sil (şifre dahil)
    if (SPIFFS.exists(SYSTEM_CONFIG_PATH)) {
        SPIFFS.remove(SYSTEM_CONFIG_PATH);
    }
    // First setup flag'ini sil
    if (SPIFFS.exists(FIRST_SETUP_FLAG)) {
        SPIFFS.remove(FIRST_SETUP_FLAG);
    }
    
    Serial.println("Factory reset completed. Restarting...");
    delay(1000);
    ESP.restart();
}

String FlexKey::getChipID() {
    return chipID;
}

String FlexKey::getAPSSID() {
    return apSSID;
}

String FlexKey::getBrowserPassword() {
    return browserPassword;
}

bool FlexKey::isSystemReady() {
    return SPIFFS.exists(SYSTEM_CONFIG_PATH);
}

// New getter methods for FlexKeyWeb
bool FlexKey::getAPEnabled() {
    return apEnabled;
}

bool FlexKey::getIsAuthenticated() {
    return isAuthenticated;
}

bool FlexKey::getIsFirstSetup() {
    return isFirstTimeSetup();
}

bool FlexKey::validatePassword(const String& password) {
    String hash = sha256(password);
    return hash == browserPassword;
}

// AP modunu aç/kapat endpoint


// ===== FlexKey_RF Yardımcıları ve API (geçici) =====

String FlexKey::fxrfNormalizeUid(const String &inRaw) {
    return FlexKeyRF::Manager::normalizeUid(inRaw);
}

bool FlexKey::fxrfLoadConfig(DynamicJsonDocument &doc) {
    FlexKeyRF::Manager mgr; return mgr.loadConfig(doc);
}

bool FlexKey::fxrfSaveConfig(DynamicJsonDocument &doc) {
    FlexKeyRF::Manager mgr; return mgr.saveConfig(doc);
}

void FlexKey::fxrfComputeStats(const DynamicJsonDocument &doc, int &gCount, int &uCount, int &urlCount) {
    FlexKeyRF::Manager::computeStats(doc, gCount, uCount, urlCount);
}

