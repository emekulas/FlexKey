#include <FlexKey.h>
#include <FlexKey_Languages.h>

FlexKey flexkey;
FlexKeyLanguages languageManager;

void setup() {
  Serial.begin(115200);
  
  // Dil ayarla
  languageManager.setLanguage("tr");
  
  // FlexKey başlat
  flexkey.begin();
  
  // Test çevirisi
  LanguageTexts texts = languageManager.getTexts();
  Serial.println("FlexKey başlatıldı!");
  Serial.println("Dil: " + languageManager.getCurrentLanguage());
  Serial.println("Hoşgeldin metni: " + texts.welcome);
}

void loop() {
  // Ana döngü
  delay(1000);
}
