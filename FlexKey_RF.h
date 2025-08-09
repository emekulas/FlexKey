#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>
#include <FS.h>
#include <SPIFFS.h>

namespace FlexKeyRF {

// Limits
static constexpr int MAX_GROUPS = 10;
static constexpr int MAX_UIDS_PER_GROUP = 32;
static constexpr int MAX_URLS_PER_GROUP = 16;
static constexpr size_t MAX_URL_LENGTH = 1000;

// Config storage path (SPIFFS)
static constexpr const char* CONFIG_PATH = "/user/fxrf.json";

/*
 * Relay Contact Types:
 * NO (Normally Open): Röle normal durumda AÇIK'tır. Enerji verildiğinde KAPANIR.
 *     - Kapı kilidi uygulamaları için idealdir
 *     - Güç kesildiğinde kapı açık kalır (güvenlik riski)
 * 
 * NC (Normally Closed): Röle normal durumda KAPALI'dır. Enerji verildiğinde AÇILIR.
 *     - Güvenlik sistemleri için idealdir  
 *     - Güç kesildiğinde kapı kilitli kalır (güvenli)
 */

class Manager {
public:
    bool loadConfig(DynamicJsonDocument& doc);
    bool saveConfig(DynamicJsonDocument& doc);
    static void computeStats(const DynamicJsonDocument& doc, int& groups, int& uids, int& urls);
    static String normalizeUid(const String& raw);
};

} // namespace FlexKeyRF
