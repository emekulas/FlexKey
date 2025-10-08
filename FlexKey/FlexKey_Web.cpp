#include "FlexKey_Web.h"
#include "FlexKey_Storage.h"
#include "FlexKey_RFID.h"
#include "FlexKey_Relay.h"
#include "FlexKey_HTTP.h"
#include <ArduinoJson.h>
#include <WiFi.h>

FlexKeyWeb::FlexKeyWeb(SystemConfig_t* config, FlexKeyStorage* stor, 
                       FlexKeyRFID* rfidReader, FlexKeyRelay* relayCtrl, 
                       FlexKeyHTTP* httpClient, LastUID_t* lastUIDPtr)
    : server(nullptr), sysConfig(config), storage(stor), 
      rfid(rfidReader), relay(relayCtrl), http(httpClient), lastUID(lastUIDPtr) {
}

void FlexKeyWeb::begin() {
    server = new WebServer(WEB_SERVER_PORT);
    
    // Page routes
    server->on("/", HTTP_GET, [this]() { this->handleRoot(); });
    server->on("/style.css", HTTP_GET, [this]() { this->handleCSS(); });
    server->on("/script.js", HTTP_GET, [this]() { this->handleJS(); });
    
    // API routes
    server->on("/api/config", HTTP_GET, [this]() { this->handleAPIGetConfig(); });
    server->on("/api/groupmode", HTTP_POST, [this]() { this->handleAPISetGroupMode(); });
    server->on("/api/activegroup", HTTP_POST, [this]() { this->handleAPISetActiveGroup(); });
    server->on("/api/groups", HTTP_GET, [this]() { this->handleAPIGetGroups(); });
    server->on("/api/group/save", HTTP_POST, [this]() { this->handleAPISaveGroup(); });
    server->on("/api/group/deleteuid", HTTP_POST, [this]() { this->handleAPIDeleteUID(); });
    server->on("/api/group/adduid", HTTP_POST, [this]() { this->handleAPIAddUID(); });
    server->on("/api/wifi", HTTP_GET, [this]() { this->handleAPIGetWiFiConfig(); });
    server->on("/api/wifi/save", HTTP_POST, [this]() { this->handleAPISaveWiFiConfig(); });
    server->on("/api/system", HTTP_GET, [this]() { this->handleAPIGetSystemInfo(); });
    server->on("/api/relay/test", HTTP_POST, [this]() { this->handleAPITestRelay(); });
    server->on("/api/lastuid", HTTP_GET, [this]() { this->handleAPIGetLastUID(); });
    server->on("/api/globalrelay", HTTP_POST, [this]() { this->handleAPISaveGlobalRelay(); });
    server->on("/api/uidconflicts", HTTP_GET, [this]() { this->handleAPICheckUIDConflicts(); });
    
    server->onNotFound([this]() { this->handleNotFound(); });
    
    server->begin();
    Serial.println("[WEB] Server started on port " + String(WEB_SERVER_PORT));
}

void FlexKeyWeb::handleClient() {
    if (server) {
        server->handleClient();
    }
}

bool FlexKeyWeb::isRunning() {
    return server != nullptr;
}

String FlexKeyWeb::generateHTML() {
    String html = R"rawliteral(<!DOCTYPE html>
<html>
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>)rawliteral";
    html += SYSTEM_NAME;
    html += R"rawliteral(</title>
    <link rel="stylesheet" href="/style.css">
</head>
<body>
    <div class="container">
        <div class="header">
            <h1>FlexKey RFID Access Control</h1>
            <p class="device-id">Device ID: <span id="deviceID">Loading...</span></p>
        </div>

        <div class="tabs">
            <button class="tab active" data-tab="tab1">ID & GRUP</button>
            <button class="tab" data-tab="tab2">URL & RÖLE</button>
            <button class="tab" data-tab="tab3">BAĞLANTI</button>
            <button class="tab" data-tab="tab4">BİLGİ</button>
        </div>

        <div id="tab1" class="tab-content active">
            <div class="section">
                <h2>GRUP MODU</h2>
                <div class="toggle-container">
                    <span>TEKLİ GRUP</span>
                    <label class="switch">
                        <input type="checkbox" id="multiGroupMode">
                        <span class="slider"></span>
                    </label>
                    <span>ÇOKLU GRUP</span>
                </div>
                <div id="multiGroupWarning" class="warning" style="display:none;">
                    Çoklu grup modunda aynı UID birden fazla grupta bulundu! 
                    Bu UID okutulduğunda tüm grupların URL'leri tetiklenecektir.
                </div>
            </div>

            <div class="section" id="relaySection" style="display:none;">
                <h2>GLOBAL RÖLE AYARLARI</h2>
                <div class="form-group">
                    <label><input type="checkbox" id="globalRelayEnabled"> Röle Etkin</label>
                </div>
                <div class="form-group">
                    <label><input type="radio" name="globalRelayMode" value="toggle" checked> Toggle</label>
                    <label><input type="radio" name="globalRelayMode" value="pulse"> Pulse</label>
                </div>
                <div class="form-group">
                    <label>Pulse Süresi (ms)</label>
                    <input type="number" id="globalRelayPulse" value="500" min="100" max="10000">
                </div>
                <button class="btn" onclick="saveGlobalRelay()">KAYDET</button>
            </div>

            <div class="section" id="singleGroupSelect" style="display:none;">
                <h2>AKTİF GRUP</h2>
                <select id="activeGroup" onchange="changeActiveGroup()">
                    <option value="0">Grup 1</option>
                    <option value="1">Grup 2</option>
                    <option value="2">Grup 3</option>
                    <option value="3">Grup 4</option>
                    <option value="4">Grup 5</option>
                </select>
            </div>

            <div class="section">
                <h2>GRUP YÖNETİMİ</h2>
                <div class="group-tabs" id="groupTabs"></div>
                <div id="groupContent"></div>
            </div>
        </div>

        <!-- Tab 2: URLs & Relay -->
        <div id="tab2" class="tab-content">
            <div class="section">
                <h2>URL & RELAIS AYARLARI</h2>
                <div class="group-tabs" id="groupTabsURL"></div>
                <div id="groupURLContent"></div>
            </div>
        </div>

        <!-- Tab 3: Connection -->
        <div id="tab3" class="tab-content">
            <div class="section">
                <h2>PRIMARY WiFi</h2>
                <div class="form-group">
                    <label>SSID</label>
                    <input type="text" id="primarySSID" placeholder="WiFi adı">
                </div>
                <div class="form-group">
                    <label>Şifre</label>
                    <input type="password" id="primaryPassword" placeholder="WiFi şifresi">
                </div>
            </div>

            <div class="section">
                <h2>BACKUP WiFi</h2>
                <div class="form-group">
                    <label>SSID</label>
                    <input type="text" id="backupSSID" placeholder="Yedek WiFi">
                </div>
                <div class="form-group">
                    <label>Şifre</label>
                    <input type="password" id="backupPassword" placeholder="Yedek şifre">
                </div>
            </div>

            <div class="section">
                <h2>STATİK IP</h2>
                <div class="form-group">
                    <label><input type="checkbox" id="useStaticIP"> Statik IP Kullan</label>
                </div>
                <div class="form-group">
                    <label>IP Adresi</label>
                    <input type="text" id="staticIP" placeholder="192.168.1.100">
                </div>
                <div class="form-group">
                    <label>Gateway</label>
                    <input type="text" id="gateway" placeholder="192.168.1.1">
                </div>
                <div class="form-group">
                    <label>Subnet</label>
                    <input type="text" id="subnet" placeholder="255.255.255.0">
                </div>
                <div class="form-group">
                    <label>DNS</label>
                    <input type="text" id="dns" placeholder="8.8.8.8">
                </div>
            </div>

            <button class="btn" onclick="saveWiFiConfig()">KAYDET & RESTART</button>
            
            <div class="info-box" style="margin-top:20px;">
                <h3>NOT: AP MODE HER ZAMAN AKTİF</h3>
                <p style="color:#888; font-size:12px;">
                    AP modu devre dışı bırakılamaz. Sistem her zaman hem AP hem de WiFi modunda çalışır.
                    Bu sayede WiFi bağlantısı kesilse bile cihaza erişim devam eder.
                </p>
            </div>
        </div>

        <div id="tab4" class="tab-content">
            <div class="section">
                <h2>SİSTEM BİLGİLERİ</h2>
                <div class="info-item">
                    <span>IP Adresi:</span>
                    <span id="ipAddress">-</span>
                </div>
                <div class="info-item">
                    <span>MAC Adresi:</span>
                    <span id="macAddress">-</span>
                </div>
                <div class="info-item">
                    <span>Chip ID:</span>
                    <span id="chipID">-</span>
                </div>
                <div class="info-item">
                    <span>Toplam UID:</span>
                    <span id="totalUIDs">-</span>
                </div>
                <div class="info-item">
                    <span>Aktif Grup:</span>
                    <span id="activeGroupName">-</span>
                </div>
                <div class="info-item">
                    <span>Versiyon:</span>
                    <span>)rawliteral";
    html += FLEXKEY_VERSION;
    html += R"rawliteral(</span>
                </div>
            </div>

            <div class="section">
                <h2>SON OKUTULAN UID (60 saniye)</h2>
                <div id="lastUIDDisplay" class="last-uid-box">
                    <div id="lastUIDValue" style="font-size:20px; font-weight:bold; margin-bottom:10px;">Henüz kart okutulmadı</div>
                    <div id="lastUIDTime" style="font-size:12px; color:#888;"></div>
                </div>
            </div>

            <div class="section">
                <h2>KULLANIM KLAVUZU</h2>
                <p>
                    <strong>FlexKey RFID Access Control System</strong><br><br>
                    1. ID & GRUP sekmesinden RFID UID'leri ekleyin<br>
                    2. URL & RÖLE sekmesinden tetikleme URL'lerini ayarlayın<br>
                    3. BAĞLANTI sekmesinden WiFi yapılandırın<br><br>
                    Detaylı bilgi: <a href="https://smartkraft.ch/FlexKey" target="_blank">smartkraft.ch/FlexKey</a>
                </p>
            </div>

            <div class="section">
                <button onclick="testRelay()" style="width:100%; padding:15px; margin-bottom:10px;">RÖLE TEST</button>
                <button onclick="confirmRestart()" style="width:100%; padding:15px;">YENİDEN BAŞLAT</button>
            </div>
        </div>
    </div>
    <script src="/script.js"></script>
</body>
</html>)rawliteral";
    
    return html;
}

void FlexKeyWeb::handleRoot() {
    server->send(200, "text/html", generateHTML());
}

void FlexKeyWeb::handleCSS() {
    server->send(200, "text/css", generateCSS());
}

void FlexKeyWeb::handleJS() {
    server->send(200, "application/javascript", generateJS());
}

void FlexKeyWeb::handleNotFound() {
    server->send(404, "text/plain", "404: Not Found");
}

// CSS Generator - Simple Clean Design
String FlexKeyWeb::generateCSS() {
    return R"rawliteral(
* {
    margin: 0;
    padding: 0;
    box-sizing: border-box;
}

body {
    font-family: Arial, sans-serif;
    background: #f5f5f5;
    color: #333;
    padding: 20px;
    line-height: 1.6;
}

.container {
    max-width: 1000px;
    margin: 0 auto;
    background: white;
    border-radius: 8px;
    box-shadow: 0 2px 10px rgba(0,0,0,0.1);
    padding: 30px;
}

.header {
    text-align: center;
    border-bottom: 3px solid #007bff;
    padding-bottom: 20px;
    margin-bottom: 30px;
}

.header h1 {
    font-size: 28px;
    color: #007bff;
    margin-bottom: 10px;
}

.device-id {
    font-size: 14px;
    color: #666;
}

/* Buttons */
.btn {
    background: #007bff;
    color: white;
    border: none;
    padding: 10px 24px;
    cursor: pointer;
    font-size: 14px;
    border-radius: 4px;
    transition: background 0.3s;
}

.btn:hover {
    background: #0056b3;
}

.btn.danger {
    background: #dc3545;
}

.btn.danger:hover {
    background: #c82333;
}

.btn.warning {
    background: #ffc107;
    color: #212529;
}

.btn.warning:hover {
    background: #e0a800;
}

/* Tabs */
.tabs {
    display: grid;
    grid-template-columns: repeat(4, 1fr);
    gap: 2px;
    margin-bottom: 20px;
    background: #dee2e6;
}

.tab {
    background: white;
    color: #495057;
    border: none;
    padding: 12px;
    cursor: pointer;
    font-size: 14px;
    transition: all 0.3s;
}

.tab:hover {
    background: #e9ecef;
    color: #007bff;
}

.tab.active {
    background: #007bff;
    color: white;
    font-weight: 600;
}

/* Tab Content */
.tab-content {
    display: none;
}

.tab-content.active {
    display: block;
}

/* Section */
.section {
    border: 1px solid #dee2e6;
    padding: 20px;
    margin-bottom: 15px;
    background: white;
    border-radius: 4px;
}

.section h2 {
    font-size: 18px;
    color: #212529;
    font-size: 18px;
    color: #212529;
    margin-bottom: 15px;
    border-bottom: 2px solid #007bff;
    padding-bottom: 10px;
}

.section h3 {
    font-size: 16px;
    color: #495057;
    margin-bottom: 15px;
}

/* Form Elements */
.form-group {
    margin-bottom: 15px;
}

.form-group label {
    display: block;
    margin-bottom: 5px;
    font-size: 14px;
    color: #495057;
    font-weight: 500;
}

input[type="text"],
input[type="password"],
input[type="number"],
select,
textarea {
    width: 100%;
    background: white;
    color: #495057;
    border: 1px solid #ced4da;
    padding: 8px 12px;
    font-size: 14px;
    border-radius: 4px;
}

input[type="text"]:focus,
input[type="password"]:focus,
input[type="number"]:focus,
select:focus,
textarea:focus {
    outline: none;
    border-color: #007bff;
    box-shadow: 0 0 0 0.2rem rgba(0, 123, 255, 0.25);
}

textarea {
    resize: vertical;
    min-height: 100px;
}

/* Toggle Switch */
.toggle-container {
    display: flex;
    align-items: center;
    gap: 15px;
    margin: 15px 0;
    padding: 15px;
    background: #f8f9fa;
    border: 1px solid #dee2e6;
    border-radius: 4px;
}

.toggle-container span {
    font-size: 14px;
}

.switch {
    position: relative;
    display: inline-block;
    width: 60px;
    height: 30px;
}

.switch input {
    opacity: 0;
    width: 0;
    height: 0;
}

.slider {
    position: absolute;
    cursor: pointer;
    top: 0;
    left: 0;
    right: 0;
    bottom: 0;
    background-color: #ccc;
    border-radius: 30px;
    transition: .3s;
}

.slider:before {
    position: absolute;
    content: "";
    height: 22px;
    width: 22px;
    left: 4px;
    bottom: 4px;
    background-color: white;
    border-radius: 50%;
    transition: .3s;
}

input:checked + .slider {
    background-color: #007bff;
}

input:checked + .slider:before {
    transform: translateX(30px);
}

/* Warning Box */
.warning {
    background: #fff3cd;
    border: 1px solid #ffc107;
    color: #856404;
    padding: 12px;
    margin: 15px 0;
    font-size: 14px;
    border-radius: 4px;
}

.warning::before {
    content: "⚠️ ";
}

/* Info Box */
.info-box {
    background: #e7f3ff;
    border: 1px solid #b3d7ff;
    padding: 15px;
    margin-bottom: 15px;
    border-radius: 4px;
}

.info-item {
    display: flex;
    justify-content: space-between;
    padding: 8px 0;
    border-bottom: 1px solid #d0e7ff;
    font-size: 14px;
}

.info-item:last-child {
    border-bottom: none;
}

.info-item span:first-child {
    color: #495057;
    font-weight: 500;
}

.info-item span:last-child {
.info-item span:last-child {
    color: #007bff;
    font-weight: 600;
}

/* Info Text */
.info-text {
    padding: 15px;
    background: #e7f3ff;
    border: 1px solid #b3d7ff;
    border-radius: 4px;
    color: #004085;
    font-size: 14px;
    line-height: 1.6;
}

.info-text .highlight {
    color: #dc3545;
    font-weight: 600;
}

/* UID List */
.uid-list {
    max-height: 300px;
    overflow-y: auto;
    border: 1px solid #dee2e6;
    padding: 10px;
    margin-bottom: 15px;
    background: #f8f9fa;
    border-radius: 4px;
}

.uid-item {
    display: flex;
    justify-content: space-between;
    align-items: center;
    padding: 10px;
    border-bottom: 1px solid #dee2e6;
    transition: background 0.2s;
    background: white;
    margin-bottom: 5px;
    border-radius: 3px;
}

.uid-item:hover {
    background: #e9ecef;
}

.uid-item:last-child {
    border-bottom: none;
}

.uid-item .mono {
    font-family: 'Consolas', 'Monaco', monospace;
    color: #007bff;
    font-size: 14px;
    font-weight: 600;
}

/* Group Tabs */
.group-tabs {
    display: flex;
    gap: 5px;
    margin-bottom: 15px;
    flex-wrap: wrap;
}

.group-tab {
    background: white;
    color: #495057;
    border: 1px solid #dee2e6;
    padding: 8px 16px;
    cursor: pointer;
    font-size: 14px;
    border-radius: 4px;
    transition: all 0.3s;
}

.group-tab:hover {
    background: #e9ecef;
    border-color: #007bff;
    color: #007bff;
}

.group-tab.active {
    background: #007bff;
    color: white;
    border-color: #007bff;
    font-weight: 600;
}

/* Scrollbar */
::-webkit-scrollbar {
    width: 8px;
    height: 8px;
}

::-webkit-scrollbar-track {
    background: #f1f1f1;
}

::-webkit-scrollbar-thumb {
    background: #888;
    border-radius: 4px;
}

::-webkit-scrollbar-thumb:hover {
    background: #555;
}

/* Responsive */
@media (max-width: 768px) {
    .header h1 {
        font-size: 22px;
    }
    
    .tabs {
        grid-template-columns: repeat(2, 1fr);
    }
}

.mono {
    font-family: 'Consolas', 'Monaco', monospace;
    color: #007bff;
    font-weight: 600;
}
)rawliteral";
}

// JavaScript generation
String FlexKeyWeb::generateJS() {
    return R"rawliteral(
let currentGroupTab = 0;
let currentURLTab = 0;
let config = {};

// Initialize on load
document.addEventListener('DOMContentLoaded', function() {
    initTabs();
    loadConfig();
    loadSystemInfo();
    startLastUIDPolling();
    checkUIDConflicts();
});

// Tab switching
function initTabs() {
    const tabs = document.querySelectorAll('.tab');
    tabs.forEach(tab => {
        tab.addEventListener('click', function() {
            const tabId = this.getAttribute('data-tab');
            switchTab(tabId);
        });
    });
}

function switchTab(tabId) {
    document.querySelectorAll('.tab').forEach(t => t.classList.remove('active'));
    document.querySelectorAll('.tab-content').forEach(t => t.classList.remove('active'));
    
    document.querySelector(`[data-tab="${tabId}"]`).classList.add('active');
    document.getElementById(tabId).classList.add('active');
}

// Load configuration
async function loadConfig() {
    try {
        const response = await fetch('/api/config');
        config = await response.json();
        
        document.getElementById('deviceID').textContent = config.deviceID;
        document.getElementById('multiGroupMode').checked = config.multiGroupMode;
        updateGroupModeUI();
        
        if (config.multiGroupMode) {
            loadGlobalRelay();
        }
        
        renderGroupTabs();
        renderGroupURLTabs();
        loadWiFiConfig();
    } catch (error) {
        console.error('Config load error:', error);
    }
}

// Multi-group mode toggle
document.getElementById('multiGroupMode').addEventListener('change', async function() {
    const enabled = this.checked;
    try {
        const response = await fetch('/api/groupmode', {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify({ enabled })
        });
        if (response.ok) {
            config.multiGroupMode = enabled;
            updateGroupModeUI();
            if (enabled) {
                checkUIDConflicts();
            }
        }
    } catch (error) {
        console.error('Group mode error:', error);
    }
});

function updateGroupModeUI() {
    const singleSelect = document.getElementById('singleGroupSelect');
    const relaySection = document.getElementById('relaySection');
    
    if (config.multiGroupMode) {
        singleSelect.style.display = 'none';
        relaySection.style.display = 'block';
    } else {
        singleSelect.style.display = 'block';
        relaySection.style.display = 'none';
        document.getElementById('activeGroup').value = config.activeGroupIndex;
    }
}

async function changeActiveGroup() {
    const groupIndex = parseInt(document.getElementById('activeGroup').value);
    try {
        await fetch('/api/activegroup', {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify({ groupIndex })
        });
        config.activeGroupIndex = groupIndex;
    } catch (error) {
        console.error('Active group error:', error);
    }
}

// Group tabs rendering
function renderGroupTabs() {
    const container = document.getElementById('groupTabs');
    container.innerHTML = '';
    
    for (let i = 0; i < 5; i++) {
        const btn = document.createElement('button');
        btn.className = 'group-tab' + (i === 0 ? ' active' : '');
        btn.textContent = `GRUP ${i + 1}`;
        btn.onclick = () => selectGroupTab(i);
        container.appendChild(btn);
    }
    
    selectGroupTab(0);
}

function renderGroupURLTabs() {
    const container = document.getElementById('groupTabsURL');
    container.innerHTML = '';
    
    for (let i = 0; i < 5; i++) {
        const btn = document.createElement('button');
        btn.className = 'group-tab' + (i === 0 ? ' active' : '');
        btn.textContent = `GRUP ${i + 1}`;
        btn.onclick = () => selectURLTab(i);
        container.appendChild(btn);
    }
    
    selectURLTab(0);
}

function selectGroupTab(index) {
    currentGroupTab = index;
    document.querySelectorAll('#groupTabs .group-tab').forEach((tab, i) => {
        tab.classList.toggle('active', i === index);
    });
    renderGroupContent(index);
}

function selectURLTab(index) {
    currentURLTab = index;
    document.querySelectorAll('#groupTabsURL .group-tab').forEach((tab, i) => {
        tab.classList.toggle('active', i === index);
    });
    renderURLContent(index);
}

// Group content rendering
async function renderGroupContent(groupIndex) {
    const container = document.getElementById('groupContent');
    const response = await fetch(`/api/groups`);
    const groups = await response.json();
    const group = groups[groupIndex];
    
    let html = `<h3>${group.name}</h3>`;
    html += `<div class="form-group">
                <label>Grup Adı</label>
                <input type="text" id="groupName${groupIndex}" value="${group.name}">
             </div>`;
    
    html += `<div class="uid-list">`;
    if (group.uids.length === 0) {
        html += '<div style="color:#666;padding:10px;">UID yok</div>';
    } else {
        group.uids.forEach((uid, i) => {
            html += `<div class="uid-item">
                        <span class="mono">${uid}</span>
                        <button class="danger" onclick="deleteUID(${groupIndex}, ${i})">SİL</button>
                     </div>`;
        });
    }
    html += `</div>`;
    
    html += `<div class="form-group">
                <label>UID Ekle (manuel)</label>
                <input type="text" id="newUID${groupIndex}" placeholder="AA:BB:CC:DD veya AA:BB:CC:DD:EE:FF:GG">
             </div>`;
    html += `<button onclick="addUID(${groupIndex})">UID EKLE</button>`;
    html += `<button onclick="saveGroup(${groupIndex})">KAYDET</button>`;
    
    container.innerHTML = html;
}

async function renderURLContent(groupIndex) {
    const container = document.getElementById('groupURLContent');
    const response = await fetch(`/api/groups`);
    const groups = await response.json();
    const group = groups[groupIndex];
    
    let html = `<h3>${group.name} - URL & RÖLE AYARLARI</h3>`;
    
    // Relay settings (only in single group mode)
    if (!config.multiGroupMode) {
        html += `<div class="section">
                    <h3>RÖLE AYARLARI</h3>
                    <div class="form-group">
                        <label><input type="checkbox" id="relayEnabled${groupIndex}" ${group.relayEnabled ? 'checked' : ''}> Röle Etkin</label>
                    </div>
                    <div class="form-group">
                        <label><input type="radio" name="relayMode${groupIndex}" value="toggle" ${group.relayToggle ? 'checked' : ''}> Toggle</label>
                        <label><input type="radio" name="relayMode${groupIndex}" value="pulse" ${!group.relayToggle ? 'checked' : ''}> Pulse</label>
                    </div>
                    <div class="form-group">
                        <label>Pulse Süresi (ms)</label>
                        <input type="number" id="relayPulse${groupIndex}" value="${group.relayPulseDuration}" min="100" max="10000">
                    </div>
                    <button onclick="testRelay()">TEST RÖLE</button>
                 </div>`;
    }
    
    // URLs
    html += `<div class="section"><h3>URL LİSTESİ (Max 15)</h3>`;
    for (let i = 0; i < 15; i++) {
        const url = group.urls[i] || '';
        html += `<div class="form-group">
                    <label>URL ${i + 1}</label>
                    <input type="text" id="url${groupIndex}_${i}" value="${url}" placeholder="http:// veya https://">
                 </div>`;
    }
    html += `<button onclick="saveURLs(${groupIndex})">KAYDET</button>`;
    html += `</div>`;
    
    container.innerHTML = html;
}

async function saveGroup(groupIndex) {
    const name = document.getElementById(`groupName${groupIndex}`).value;
    try {
        await fetch('/api/group/save', {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify({ groupIndex, name })
        });
        alert('Grup kaydedildi');
        renderGroupContent(groupIndex);
    } catch (error) {
        alert('Kayıt hatası: ' + error);
    }
}

async function addUID(groupIndex) {
    const uidInput = document.getElementById(`newUID${groupIndex}`).value.trim();
    if (!uidInput) {
        alert('UID giriniz');
        return;
    }
    
    try {
        const response = await fetch('/api/group/adduid', {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify({ groupIndex, uid: uidInput })
        });
        
        const result = await response.json();
        if (result.success) {
            alert('UID eklendi');
            document.getElementById(`newUID${groupIndex}`).value = '';
            renderGroupContent(groupIndex);
            checkUIDConflicts();
        } else {
            alert('Hata: ' + result.message);
        }
    } catch (error) {
        alert('Ekleme hatası: ' + error);
    }
}

async function deleteUID(groupIndex, uidIndex) {
    if (!confirm('UID silinsin mi?')) return;
    
    try {
        await fetch('/api/group/deleteuid', {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify({ groupIndex, uidIndex })
        });
        alert('UID silindi');
        renderGroupContent(groupIndex);
        checkUIDConflicts();
    } catch (error) {
        alert('Silme hatası: ' + error);
    }
}

async function saveURLs(groupIndex) {
    const urls = [];
    for (let i = 0; i < 15; i++) {
        const url = document.getElementById(`url${groupIndex}_${i}`).value.trim();
        if (url) urls.push(url);
    }
    
    // Get relay settings if single mode
    let relayEnabled = false;
    let relayToggle = true;
    let relayPulse = 500;
    
    if (!config.multiGroupMode) {
        relayEnabled = document.getElementById(`relayEnabled${groupIndex}`).checked;
        relayToggle = document.querySelector(`input[name="relayMode${groupIndex}"]:checked`).value === 'toggle';
        relayPulse = parseInt(document.getElementById(`relayPulse${groupIndex}`).value);
    }
    
    try {
        await fetch('/api/group/save', {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify({ 
                groupIndex, 
                urls, 
                relayEnabled, 
                relayToggle, 
                relayPulseDuration: relayPulse 
            })
        });
        alert('Ayarlar kaydedildi');
    } catch (error) {
        alert('Kayıt hatası: ' + error);
    }
}

async function loadGlobalRelay() {
    try {
        const response = await fetch('/api/config');
        const data = await response.json();
        document.getElementById('globalRelayEnabled').checked = data.globalRelay.enabled;
        document.querySelector(`input[name="globalRelayMode"][value="${data.globalRelay.toggle ? 'toggle' : 'pulse'}"]`).checked = true;
        document.getElementById('globalRelayPulse').value = data.globalRelay.pulseDuration;
    } catch (error) {
        console.error('Global relay load error:', error);
    }
}

async function saveGlobalRelay() {
    const enabled = document.getElementById('globalRelayEnabled').checked;
    const toggle = document.querySelector('input[name="globalRelayMode"]:checked').value === 'toggle';
    const pulse = parseInt(document.getElementById('globalRelayPulse').value);
    
    try {
        await fetch('/api/globalrelay', {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify({ enabled, toggle, pulseDuration: pulse })
        });
        alert('Global röle ayarları kaydedildi');
    } catch (error) {
        alert('Kayıt hatası: ' + error);
    }
}

async function testRelay() {
    try {
        await fetch('/api/relay/test', { method: 'POST' });
        alert('Röle test edildi');
    } catch (error) {
        alert('Test hatası: ' + error);
    }
}

// WiFi configuration
async function loadWiFiConfig() {
    try {
        const response = await fetch('/api/wifi');
        const wifi = await response.json();
        
        // Primary Network
        document.getElementById('primarySSID').value = wifi.primarySSID || '';
        document.getElementById('primaryPassword').value = wifi.primaryPassword || '';
        document.getElementById('primaryUseStaticIP').checked = wifi.primaryUseStaticIP || false;
        document.getElementById('primaryStaticIP').value = wifi.primaryStaticIP || '192.168.1.100';
        document.getElementById('primaryGateway').value = wifi.primaryGateway || '192.168.1.1';
        document.getElementById('primarySubnet').value = wifi.primarySubnet || '255.255.255.0';
        document.getElementById('primaryDNS').value = wifi.primaryDNS || '8.8.8.8';
        
        // Backup Network
        document.getElementById('backupSSID').value = wifi.backupSSID || '';
        document.getElementById('backupPassword').value = wifi.backupPassword || '';
        document.getElementById('backupUseStaticIP').checked = wifi.backupUseStaticIP || false;
        document.getElementById('backupStaticIP').value = wifi.backupStaticIP || '192.168.2.100';
        document.getElementById('backupGateway').value = wifi.backupGateway || '192.168.2.1';
        document.getElementById('backupSubnet').value = wifi.backupSubnet || '255.255.255.0';
        document.getElementById('backupDNS').value = wifi.backupDNS || '8.8.8.8';
        
        // AP Mode
        document.getElementById('apModeEnabled').checked = wifi.apModeEnabled !== false;
        
    } catch (error) {
        console.error('WiFi config load error:', error);
    }
}

async function saveWiFiConfig() {
    if (!confirm('WiFi ayarları kaydedilecek ve sistem yeniden başlatılacak. Devam edilsin mi?')) {
        return;
    }
    
    const primarySSID = document.getElementById('primarySSID').value.trim();
    const backupSSID = document.getElementById('backupSSID').value.trim();
    const apModeEnabled = document.getElementById('apModeEnabled').checked;
    
    // Validation: If AP mode disabled, WiFi must be configured
    if (!apModeEnabled && primarySSID === '' && backupSSID === '') {
        alert('⚠️ UYARI: AP modu kapatılamaz! En az bir WiFi SSID ayarlanmalıdır.\n\nAP modunu kapatmak için Primary veya Backup WiFi SSID girmelisiniz.');
        return;
    }
    
    const wifi = {
        // Primary Network
        primarySSID: primarySSID,
        primaryPassword: document.getElementById('primaryPassword').value,
        primaryUseStaticIP: document.getElementById('primaryUseStaticIP').checked,
        primaryStaticIP: document.getElementById('primaryStaticIP').value,
        primaryGateway: document.getElementById('primaryGateway').value,
        primarySubnet: document.getElementById('primarySubnet').value,
        primaryDNS: document.getElementById('primaryDNS').value,
        
        // Backup Network
        backupSSID: backupSSID,
        backupPassword: document.getElementById('backupPassword').value,
        backupUseStaticIP: document.getElementById('backupUseStaticIP').checked,
        backupStaticIP: document.getElementById('backupStaticIP').value,
        backupGateway: document.getElementById('backupGateway').value,
        backupSubnet: document.getElementById('backupSubnet').value,
        backupDNS: document.getElementById('backupDNS').value,
        
        // AP Mode
        apModeEnabled: apModeEnabled
    };
    
    try {
        await fetch('/api/wifi/save', {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify(wifi)
        });
        alert('Ayarlar kaydedildi. Sistem yeniden başlatılıyor...');
        setTimeout(() => location.reload(), 3000);
    } catch (error) {
        alert('Kayıt hatası: ' + error);
    }
}

// System info
async function loadSystemInfo() {
    try {
        const response = await fetch('/api/system');
        const info = await response.json();
        
        document.getElementById('ipAddress').textContent = info.ip;
        document.getElementById('macAddress').textContent = info.mac;
        document.getElementById('totalUIDs').textContent = info.totalUIDs;
        document.getElementById('activeGroupName').textContent = info.activeGroupName;
    } catch (error) {
        console.error('System info error:', error);
    }
}

// Last UID polling
function startLastUIDPolling() {
    setInterval(async () => {
        try {
            const response = await fetch('/api/lastuid');
            const data = await response.json();
            
            const elem = document.getElementById('lastUID');
            if (data.valid) {
                elem.textContent = data.uid;
                elem.style.color = '#0f0';
            } else {
                elem.textContent = 'Bekleniyor...';
                elem.style.color = '#666';
            }
        } catch (error) {
            console.error('Last UID error:', error);
        }
    }, 1000);
}

// UID conflict checking
async function checkUIDConflicts() {
    if (!config.multiGroupMode) {
        document.getElementById('multiGroupWarning').style.display = 'none';
        return;
    }
    
    try {
        const response = await fetch('/api/uidconflicts');
        const data = await response.json();
        
        const warning = document.getElementById('multiGroupWarning');
        if (data.hasConflicts) {
            warning.style.display = 'block';
        } else {
            warning.style.display = 'none';
        }
    } catch (error) {
        console.error('UID conflict check error:', error);
    }
}
)rawliteral";
}

// API Handlers implementation

void FlexKeyWeb::handleAPIGetConfig() {
    JsonDocument doc;
    
    doc["multiGroupMode"] = sysConfig->multiGroupMode;
    doc["activeGroupIndex"] = sysConfig->activeGroupIndex;
    doc["deviceID"] = sysConfig->deviceID;
    
    // Global relay
    JsonObject globalRelay = doc["globalRelay"].to<JsonObject>();
    globalRelay["enabled"] = sysConfig->globalRelay.enabled;
    globalRelay["toggle"] = sysConfig->globalRelay.toggle;
    globalRelay["pulseDuration"] = sysConfig->globalRelay.pulseDuration;
    
    String output;
    serializeJson(doc, output);
    server->send(200, "application/json", output);
}

void FlexKeyWeb::handleAPISetGroupMode() {
    if (!server->hasArg("plain")) {
        server->send(400, "application/json", "{\"error\":\"No data\"}");
        return;
    }
    
    JsonDocument doc;
    deserializeJson(doc, server->arg("plain"));
    
    bool enabled = doc["enabled"];
    sysConfig->multiGroupMode = enabled;
    storage->saveMultiGroupMode(enabled);
    
    server->send(200, "application/json", "{\"success\":true}");
}

void FlexKeyWeb::handleAPISetActiveGroup() {
    if (!server->hasArg("plain")) {
        server->send(400, "application/json", "{\"error\":\"No data\"}");
        return;
    }
    
    JsonDocument doc;
    deserializeJson(doc, server->arg("plain"));
    
    uint8_t groupIndex = doc["groupIndex"];
    if (groupIndex < MAX_GROUPS) {
        sysConfig->activeGroupIndex = groupIndex;
        storage->saveActiveGroup(groupIndex);
        server->send(200, "application/json", "{\"success\":true}");
    } else {
        server->send(400, "application/json", "{\"error\":\"Invalid group\"}");
    }
}

void FlexKeyWeb::handleAPIGetGroups() {
    JsonDocument doc;
    JsonArray groupsArray = doc.to<JsonArray>();
    
    for (uint8_t i = 0; i < MAX_GROUPS; i++) {
        JsonObject group = groupsArray.add<JsonObject>();
        group["name"] = sysConfig->groups[i].name;
        group["active"] = sysConfig->groups[i].active;
        group["relayEnabled"] = sysConfig->groups[i].relayEnabled;
        group["relayToggle"] = sysConfig->groups[i].relayToggle;
        group["relayPulseDuration"] = sysConfig->groups[i].relayPulseDuration;
        
        // UIDs
        JsonArray uids = group["uids"].to<JsonArray>();
        for (uint16_t j = 0; j < sysConfig->groups[i].uidCount; j++) {
            uids.add(sysConfig->groups[i].uids[j].toString());
        }
        
        // URLs
        JsonArray urls = group["urls"].to<JsonArray>();
        for (uint8_t j = 0; j < sysConfig->groups[i].urlCount; j++) {
            urls.add(sysConfig->groups[i].urls[j]);
        }
    }
    
    String output;
    serializeJson(doc, output);
    server->send(200, "application/json", output);
}

void FlexKeyWeb::handleAPISaveGroup() {
    if (!server->hasArg("plain")) {
        server->send(400, "application/json", "{\"error\":\"No data\"}");
        return;
    }
    
    JsonDocument doc;
    deserializeJson(doc, server->arg("plain"));
    
    uint8_t groupIndex = doc["groupIndex"];
    if (groupIndex >= MAX_GROUPS) {
        server->send(400, "application/json", "{\"error\":\"Invalid group\"}");
        return;
    }
    
    // Update group name if provided
    if (doc.containsKey("name")) {
        sysConfig->groups[groupIndex].name = doc["name"].as<String>();
    }
    
    // Update URLs if provided
    if (doc.containsKey("urls")) {
        JsonArray urlsArray = doc["urls"];
        sysConfig->groups[groupIndex].urlCount = 0;
        for (uint8_t i = 0; i < urlsArray.size() && i < MAX_URLS_PER_GROUP; i++) {
            sysConfig->groups[groupIndex].urls[i] = urlsArray[i].as<String>();
            sysConfig->groups[groupIndex].urlCount++;
        }
    }
    
    // Update relay settings if provided (single group mode)
    if (doc.containsKey("relayEnabled")) {
        sysConfig->groups[groupIndex].relayEnabled = doc["relayEnabled"];
    }
    if (doc.containsKey("relayToggle")) {
        sysConfig->groups[groupIndex].relayToggle = doc["relayToggle"];
    }
    if (doc.containsKey("relayPulseDuration")) {
        sysConfig->groups[groupIndex].relayPulseDuration = doc["relayPulseDuration"];
    }
    
    storage->saveGroup(groupIndex, sysConfig->groups[groupIndex]);
    server->send(200, "application/json", "{\"success\":true}");
}

void FlexKeyWeb::handleAPIDeleteUID() {
    if (!server->hasArg("plain")) {
        server->send(400, "application/json", "{\"error\":\"No data\"}");
        return;
    }
    
    JsonDocument doc;
    deserializeJson(doc, server->arg("plain"));
    
    uint8_t groupIndex = doc["groupIndex"];
    uint16_t uidIndex = doc["uidIndex"];
    
    if (groupIndex >= MAX_GROUPS || uidIndex >= sysConfig->groups[groupIndex].uidCount) {
        server->send(400, "application/json", "{\"error\":\"Invalid indices\"}");
        return;
    }
    
    // Shift UIDs down
    for (uint16_t i = uidIndex; i < sysConfig->groups[groupIndex].uidCount - 1; i++) {
        sysConfig->groups[groupIndex].uids[i] = sysConfig->groups[groupIndex].uids[i + 1];
    }
    sysConfig->groups[groupIndex].uidCount--;
    
    storage->saveGroup(groupIndex, sysConfig->groups[groupIndex]);
    server->send(200, "application/json", "{\"success\":true}");
}

void FlexKeyWeb::handleAPIAddUID() {
    if (!server->hasArg("plain")) {
        server->send(400, "application/json", "{\"error\":\"No data\"}");
        return;
    }
    
    JsonDocument doc;
    deserializeJson(doc, server->arg("plain"));
    
    uint8_t groupIndex = doc["groupIndex"];
    String uidStr = doc["uid"].as<String>();
    
    if (groupIndex >= MAX_GROUPS) {
        server->send(400, "application/json", "{\"error\":\"Invalid group\"}");
        return;
    }
    
    if (sysConfig->groups[groupIndex].uidCount >= MAX_UIDS) {
        server->send(400, "application/json", "{\"error\":\"UID limit reached\"}");
        return;
    }
    
    UID_t newUID;
    if (!newUID.fromString(uidStr)) {
        server->send(400, "application/json", "{\"error\":\"Invalid UID format\"}");
        return;
    }
    
    // Check for duplicates in same group
    for (uint16_t i = 0; i < sysConfig->groups[groupIndex].uidCount; i++) {
        if (sysConfig->groups[groupIndex].uids[i].equals(newUID)) {
            server->send(400, "application/json", "{\"error\":\"UID already exists in this group\"}");
            return;
        }
    }
    
    // Add UID
    sysConfig->groups[groupIndex].uids[sysConfig->groups[groupIndex].uidCount] = newUID;
    sysConfig->groups[groupIndex].uidCount++;
    
    storage->saveGroup(groupIndex, sysConfig->groups[groupIndex]);
    server->send(200, "application/json", "{\"success\":true}");
}

void FlexKeyWeb::handleAPIGetWiFiConfig() {
    JsonDocument doc;
    
    // Primary Network
    doc["primarySSID"] = sysConfig->wifi.primarySSID;
    doc["primaryPassword"] = sysConfig->wifi.primaryPassword;
    doc["primaryUseStaticIP"] = sysConfig->wifi.primaryUseStaticIP;
    doc["primaryStaticIP"] = sysConfig->wifi.primaryStaticIP.toString();
    doc["primaryGateway"] = sysConfig->wifi.primaryGateway.toString();
    doc["primarySubnet"] = sysConfig->wifi.primarySubnet.toString();
    doc["primaryDNS"] = sysConfig->wifi.primaryDNS.toString();
    
    // Backup Network
    doc["backupSSID"] = sysConfig->wifi.backupSSID;
    doc["backupPassword"] = sysConfig->wifi.backupPassword;
    doc["backupUseStaticIP"] = sysConfig->wifi.backupUseStaticIP;
    doc["backupStaticIP"] = sysConfig->wifi.backupStaticIP.toString();
    doc["backupGateway"] = sysConfig->wifi.backupGateway.toString();
    doc["backupSubnet"] = sysConfig->wifi.backupSubnet.toString();
    doc["backupDNS"] = sysConfig->wifi.backupDNS.toString();
    
    // AP Mode
    doc["apModeEnabled"] = sysConfig->wifi.apModeEnabled;
    
    String output;
    serializeJson(doc, output);
    server->send(200, "application/json", output);
}

void FlexKeyWeb::handleAPISaveWiFiConfig() {
    if (!server->hasArg("plain")) {
        server->send(400, "application/json", "{\"error\":\"No data\"}");
        return;
    }
    
    JsonDocument doc;
    deserializeJson(doc, server->arg("plain"));
    
    // Primary Network
    sysConfig->wifi.primarySSID = doc["primarySSID"].as<String>();
    sysConfig->wifi.primaryPassword = doc["primaryPassword"].as<String>();
    sysConfig->wifi.primaryUseStaticIP = doc["primaryUseStaticIP"];
    sysConfig->wifi.primaryStaticIP.fromString(doc["primaryStaticIP"].as<String>());
    sysConfig->wifi.primaryGateway.fromString(doc["primaryGateway"].as<String>());
    sysConfig->wifi.primarySubnet.fromString(doc["primarySubnet"].as<String>());
    sysConfig->wifi.primaryDNS.fromString(doc["primaryDNS"].as<String>());
    
    // Backup Network
    sysConfig->wifi.backupSSID = doc["backupSSID"].as<String>();
    sysConfig->wifi.backupPassword = doc["backupPassword"].as<String>();
    sysConfig->wifi.backupUseStaticIP = doc["backupUseStaticIP"];
    sysConfig->wifi.backupStaticIP.fromString(doc["backupStaticIP"].as<String>());
    sysConfig->wifi.backupGateway.fromString(doc["backupGateway"].as<String>());
    sysConfig->wifi.backupSubnet.fromString(doc["backupSubnet"].as<String>());
    sysConfig->wifi.backupDNS.fromString(doc["backupDNS"].as<String>());
    
    // AP Mode
    sysConfig->wifi.apModeEnabled = doc["apModeEnabled"];
    
    storage->saveWiFiConfig(sysConfig->wifi);
    server->send(200, "application/json", "{\"success\":true}");
    
    delay(1000);
    ESP.restart();
}

void FlexKeyWeb::handleAPIGetSystemInfo() {
    JsonDocument doc;
    
    doc["ip"] = WiFi.localIP().toString();
    doc["mac"] = WiFi.macAddress();
    
    // Count total UIDs
    uint16_t totalUIDs = 0;
    for (uint8_t i = 0; i < MAX_GROUPS; i++) {
        totalUIDs += sysConfig->groups[i].uidCount;
    }
    doc["totalUIDs"] = totalUIDs;
    
    // Active group name
    if (sysConfig->multiGroupMode) {
        doc["activeGroupName"] = "Çoklu Grup Modu";
    } else {
        doc["activeGroupName"] = sysConfig->groups[sysConfig->activeGroupIndex].name;
    }
    
    String output;
    serializeJson(doc, output);
    server->send(200, "application/json", output);
}

void FlexKeyWeb::handleAPITestRelay() {
    relay->pulse(500);
    server->send(200, "application/json", "{\"success\":true}");
}

void FlexKeyWeb::handleAPIGetLastUID() {
    JsonDocument doc;
    
    if (lastUID->isExpired() || !lastUID->uid.isValid) {
        doc["valid"] = false;
        doc["uid"] = "";
    } else {
        doc["valid"] = true;
        doc["uid"] = lastUID->uid.toString();
    }
    
    String output;
    serializeJson(doc, output);
    server->send(200, "application/json", output);
}

void FlexKeyWeb::handleAPISaveGlobalRelay() {
    if (!server->hasArg("plain")) {
        server->send(400, "application/json", "{\"error\":\"No data\"}");
        return;
    }
    
    JsonDocument doc;
    deserializeJson(doc, server->arg("plain"));
    
    sysConfig->globalRelay.enabled = doc["enabled"];
    sysConfig->globalRelay.toggle = doc["toggle"];
    sysConfig->globalRelay.pulseDuration = doc["pulseDuration"];
    
    storage->saveGlobalRelay(sysConfig->globalRelay);
    server->send(200, "application/json", "{\"success\":true}");
}

void FlexKeyWeb::handleAPICheckUIDConflicts() {
    JsonDocument doc;
    String message;
    bool hasConflicts = checkUIDConflicts(message);
    
    doc["hasConflicts"] = hasConflicts;
    doc["message"] = message;
    
    String output;
    serializeJson(doc, output);
    server->send(200, "application/json", output);
}

// Helper functions

String FlexKeyWeb::jsonEscape(const String& str) {
    String escaped = str;
    escaped.replace("\\", "\\\\");
    escaped.replace("\"", "\\\"");
    escaped.replace("\n", "\\n");
    escaped.replace("\r", "\\r");
    escaped.replace("\t", "\\t");
    return escaped;
}

bool FlexKeyWeb::checkUIDConflicts(String& conflictMessage) {
    if (!sysConfig->multiGroupMode) {
        return false;
    }
    
    bool hasConflicts = false;
    conflictMessage = "";
    
    // Check each UID across all groups
    for (uint8_t g1 = 0; g1 < MAX_GROUPS; g1++) {
        for (uint16_t u1 = 0; u1 < sysConfig->groups[g1].uidCount; u1++) {
            int conflictCount = 0;
            
            for (uint8_t g2 = 0; g2 < MAX_GROUPS; g2++) {
                if (g1 == g2) continue;
                
                for (uint16_t u2 = 0; u2 < sysConfig->groups[g2].uidCount; u2++) {
                    if (sysConfig->groups[g1].uids[u1].equals(sysConfig->groups[g2].uids[u2])) {
                        conflictCount++;
                        hasConflicts = true;
                    }
                }
            }
            
            if (conflictCount > 0) {
                conflictMessage += sysConfig->groups[g1].uids[u1].toString() + " (" + String(conflictCount + 1) + " grup), ";
            }
        }
    }
    
    return hasConflicts;
}

