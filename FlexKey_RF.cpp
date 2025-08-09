#include "FlexKey_RF.h"

namespace FlexKeyRF {

bool Manager::loadConfig(DynamicJsonDocument& doc) {
    if (!SPIFFS.exists(CONFIG_PATH)) {
        doc.clear();
        doc.createNestedArray("groups");
        // NO (Normally Open): Röle normalde açık, enerji verildiğinde kapanır
        // NC (Normally Closed): Röle normalde kapalı, enerji verildiğinde açılır
        doc["relayMode"] = "NO"; // NO or NC - Relay default contact type
        return true;
    }
    File f = SPIFFS.open(CONFIG_PATH, "r");
    if (!f) return false;
    auto err = deserializeJson(doc, f);
    f.close();
    return !err;
}

bool Manager::saveConfig(DynamicJsonDocument& doc) {
    File f = SPIFFS.open(CONFIG_PATH, "w");
    if (!f) return false;
    serializeJson(doc, f);
    f.close();
    return true;
}

void Manager::computeStats(const DynamicJsonDocument& doc, int& g, int& u, int& url) {
    g = u = url = 0;
    if (!doc.containsKey("groups")) return;
    if (!doc["groups"].is<JsonArrayConst>()) return;
    JsonArrayConst groups = doc["groups"].as<JsonArrayConst>();
    for (JsonObjectConst gr : groups) {
        g++;
        if (gr.containsKey("uids") && gr["uids"].is<JsonArrayConst>())
            u += gr["uids"].as<JsonArrayConst>().size();
        if (gr.containsKey("urls") && gr["urls"].is<JsonArrayConst>())
            url += gr["urls"].as<JsonArrayConst>().size();
    }
}

String Manager::normalizeUid(const String& raw) {
    String s; s.reserve(raw.length());
    for (size_t i = 0; i < raw.length(); ++i) {
        char c = raw[i];
        if (c == ':' || c == '-' || c == ' ') continue;
        if (c >= 'a' && c <= 'f') c = c - 'a' + 'A';
        bool isHex = (c >= '0' && c <= '9') || (c >= 'A' && c <= 'F');
        if (!isHex) return String("");
        s += c;
    }
    if (!(s.length() == 8 || s.length() == 14)) return String("");
    String out; out.reserve(s.length() + s.length()/2);
    for (size_t i = 0; i < s.length(); i += 2) {
        if (i) out += ':';
        out += s[i]; out += s[i+1];
    }
    return out;
}

} // namespace FlexKeyRF
