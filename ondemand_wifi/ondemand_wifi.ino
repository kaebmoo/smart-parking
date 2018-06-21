#include <ESP8266WiFi.h>

#include <ESP8266WebServer.h>
#include <DNSServer.h>
#include <WiFiManager.h> //https://github.com/tzapu/WiFiManager

#define TRIGGER_PIN D3

void setup() {
  // put your setup code here, to run once:
  pinMode(TRIGGER_PIN, INPUT);
  Serial.begin(115200);
  delay(10);

  // We start by connecting to a WiFi network

  Serial.println();
  Serial.println();
  Serial.println(WiFi.SSID());
  Serial.println(WiFi.psk());
  String SSID = WiFi.SSID();
  String PSK = WiFi.psk();
  WiFi.begin();
  Serial.print("Connecting");
  Serial.println();

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    if ( digitalRead(TRIGGER_PIN) == LOW ) {
      ondemandWiFi();
    }
  }
  Serial.println();
  Serial.print("Connected, IP address: ");
  Serial.println(WiFi.localIP()); 
}

void loop() {
  // put your main code here, to run repeatedly:
  if ( digitalRead(TRIGGER_PIN) == LOW ) {
    ondemandWiFi();
  }

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
}

void ondemandWiFi()
{
    WiFiManager wifiManager;
    if (!wifiManager.startConfigPortal("OnDemandAP")) {
      Serial.println("failed to connect and hit timeout");
      delay(3000);
      //reset and try again, or maybe put it to deep sleep
      ESP.reset();
      delay(5000);
    }
    //if you get here you have connected to the WiFi
    Serial.println("connected...yeey :)");
}

