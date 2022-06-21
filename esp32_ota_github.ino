#include <HTTPUpdate.h>
#include <WiFiClientSecure.h>
#include "cert.h"
#include <WiFiManager.h> // https://github.com/tzapu/WiFiManager

//WiFiManager
WiFiManager wm;

String FirmwareVer = {
  "2.7"
};

#define URL_fw_Version "https://raw.githubusercontent.com/EmreOnaran/ota_updater/main/bin_version.txt"
#define URL_fw_Bin "https://raw.githubusercontent.com/EmreOnaran/ota_updater/main/fw.bin"

int FirmwareVersionCheck();
void firmwareUpdate();

unsigned long previousMillis = 0;
unsigned long previousMillis_2 = 0;
const long interval = 60000;
const long mini_interval = 1000;



void setup() {
  Serial.begin(115200);

    WiFi.mode(WIFI_STA); // explicitly set mode, esp defaults to STA+AP

    bool res;
    res = wm.autoConnect("UbiHouse Mini"); 
    
    if(!res) {
        Serial.println("Internet bağlantısı kurulamadı");
        // ESP.restart();
    } 
    else {
        //if connected to the WiFi print this
        Serial.println("Internete bağlandı");
    }
    
  Serial.print("Active firmware version:");
  Serial.println(FirmwareVer);
}

void loop() {
  repeatedCall();
}



void repeatedCall() {
  static int num=0;
  unsigned long currentMillis = millis();
  if ((currentMillis - previousMillis) >= interval) {
    // save the last time you blinked the LED
    previousMillis = currentMillis;
    if (FirmwareVersionCheck()) {
      firmwareUpdate();
    }
  }
  if ((currentMillis - previousMillis_2) >= mini_interval) {
    previousMillis_2 = currentMillis;
    Serial.print("idle loop...");
    Serial.print(num++);
    Serial.print(" Active fw version:");
    Serial.println(FirmwareVer);
   if(WiFi.status() == WL_CONNECTED) 
   {
       Serial.println("wifi connected");
   }
   else
   {
      Serial.println("wifi not connected");
   }
  }
}

///// Firmware update /////

void firmwareUpdate(void) {
  WiFiClientSecure client;
  client.setCACert(rootCACertificate);
  t_httpUpdate_return ret = httpUpdate.update(client, URL_fw_Bin);

  switch (ret) {
  case HTTP_UPDATE_FAILED:
    Serial.printf("HTTP_UPDATE_FAILD Error (%d): %s\n", httpUpdate.getLastError(), httpUpdate.getLastErrorString().c_str());
    break;

  case HTTP_UPDATE_NO_UPDATES:
    Serial.println("HTTP_UPDATE_NO_UPDATES");
    break;

  case HTTP_UPDATE_OK:
    Serial.println("HTTP_UPDATE_OK");
    break;
  }
}

int FirmwareVersionCheck(void) {
  String payload;
  int httpCode;
  String fwurl = "";
  fwurl += URL_fw_Version;
  // fwurl += "?";
  // fwurl += String(rand());
  Serial.println(fwurl);
  WiFiClientSecure * client = new WiFiClientSecure;

  if (client) 
  {
    client -> setCACert(rootCACertificate);

    // Add a scoping block for HTTPClient https to make sure it is destroyed before WiFiClientSecure *client is 
    HTTPClient https;

    if (https.begin( * client, fwurl)) 
    { // HTTPS      
      Serial.print("[HTTPS] GET...\n");
      // start connection and send HTTP header
      delay(100);
      httpCode = https.GET();
      delay(100);
      if (httpCode == HTTP_CODE_OK) // if version received
      {
        payload = https.getString(); // save received version
      } else {
        Serial.print("error in downloading version file:");
        Serial.println(httpCode);
      }
      https.end();
    }
    delete client;
  }
      
  if (httpCode == HTTP_CODE_OK) // if version received
  {
    payload.trim();
    if (payload.equals(FirmwareVer)) {
      Serial.printf("\nDevice already on latest firmware version:%s\n", FirmwareVer);
      return 0;
    } 
    else 
    {
      Serial.println(payload);
      Serial.println("New firmware detected");
      return 1;
    }
  } 
  return 0;  
}

///// Firmware update /////
