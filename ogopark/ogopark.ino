/*
 * 
 * 

MIT License

Copyright (c) 2018 Pornthep Nivatyakul

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

*
*
*/

#include <Adafruit_NeoPixel.h>

#include <ESP8266WiFi.h>
#include <MicroGear.h>
#include <MQTTClient.h>
#include <PubSubClient.h>
#include <SHA1.h>
#include <AuthClient.h>
#include <debug.h>

#include <DNSServer.h>            //Local DNS Server used for redirecting all requests to the configuration portal
#include <ESP8266WebServer.h>     //Local WebServer used to serve the configuration portal
#include <WiFiManager.h>          //https://github.com/tzapu/WiFiManager WiFi Configuration Magic

#define trigPin D7
#define echoPin D6
#define TRIGGER_PIN D3
#define PIN D2

Adafruit_NeoPixel pixels = Adafruit_NeoPixel(1, PIN, NEO_GRB + NEO_KHZ800);

WiFiClient sensorClient;                        // declare a  wifi client 
MicroGear microgear(sensorClient);              // declare microgear object
PubSubClient mqttClient(sensorClient);

char *mqtt_user = "seal";
char *mqtt_password = "sealwiththekiss";
const char* mqtt_server = "db.ogonan.com";

char *myRoom = "sensor/1";
char *roomStatus = "sensor/1/status";
const int MAXRETRY = 4;
int mqtt_reconnect = 0;
int sensorState = 9;

void setup() {
  // put your setup code here, to run once:
  Serial.begin (115200);
  delay(10);
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(D4, OUTPUT);
  pinMode(TRIGGER_PIN, INPUT);

  pixels.begin(); // This initializes the NeoPixel library.
  pixels.setBrightness(64);

  pixels.setPixelColor(0, 255, 255, 0); // yellow #FFFF00
  pixels.show();
  
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
  pixels.setPixelColor(0, 0, 0, 255); // blue #0000FF
  pixels.show();
  delay(1000);
  pixels.setPixelColor(0, 0, 0, 0);
  pixels.show();
  delay(500);
  pixels.setPixelColor(0, 0, 0, 255); // blue #0000FF
  pixels.show();
  delay(1000);
  pixels.setPixelColor(0, 0, 0, 0);
  pixels.show();

  setup_mqtt();
}

void loop() {
  // put your main code here, to run repeatedly:
  long duration, distance;

  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  duration = pulseIn(echoPin, HIGH);
  distance = (duration/2) / 29.1;
  if (distance <= 15) { 
    digitalWrite(D4, LOW);
    pixels.setPixelColor(0, 255, 0, 0); // red #FF0000
    pixels.show();
    if (sensorState != 1) {
      mqttClient.publish(roomStatus, "1", true);
      sensorState = 1;
    }
  }
  else {
    digitalWrite(D4, HIGH);
    pixels.setPixelColor(0, 0, 128, 0); // green #008000
    pixels.show();
    if (sensorState != 0) {
      mqttClient.publish(roomStatus, "0", true);
      sensorState = 0;
    }
  }
  if (distance >= 300 || distance <= 0) {
    Serial.println("Out of range");
    pixels.setPixelColor(0, 255, 255, 0); // yellow #FFFF00
    pixels.show();
    if (sensorState != -1) {
      mqttClient.publish(roomStatus, "-1", true);
      sensorState = -1;
    }
  }
  else {
    Serial.print(distance);
    Serial.println(" cm");
  }
  delay(500);

    // put your main code here, to run repeatedly:
  if ( digitalRead(TRIGGER_PIN) == LOW ) {
    blinkLED(255,0,0);
    ondemandWiFi();
  }

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    if ( digitalRead(TRIGGER_PIN) == LOW ) {
      blinkLED(255,0,0);
      ondemandWiFi();
    }
  }
  if (!mqttClient.connected()) {
      reconnect();
  }
  mqttClient.loop();
}

void blinkLED(int red, int green, int blue)
{
  for (int i = 0; i < 5; i++) {
    pixels.setPixelColor(0, red, green, blue); // blue #0000FF
    pixels.show();  
    delay(350);
    pixels.setPixelColor(0, 0, 0, 0);
    pixels.show();
    delay(250);
  }
}

void ondemandWiFi()
{
    WiFiManager wifiManager;

    pixels.setPixelColor(0, 255, 0, 0); // red #FF0000
    pixels.show();
    if (!wifiManager.startConfigPortal("ogoSense")) {
      Serial.println("failed to connect and hit timeout");
      delay(3000);
      //reset and try again, or maybe put it to deep sleep
      ESP.reset();
      delay(5000);
    }
    //if you get here you have connected to the WiFi
    Serial.println("connected...yeey :)");
    pixels.setPixelColor(0, 0, 0, 255); // blue #0000FF
    pixels.show();
    delay(1000);
    pixels.setPixelColor(0, 0, 0, 0);
    pixels.show();
    delay(500);
    pixels.setPixelColor(0, 0, 0, 255); // blue #0000FF
    pixels.show();
    delay(1000);
    pixels.setPixelColor(0, 0, 0, 0);
    pixels.show();
}

void setup_mqtt() {

  mqttClient.setServer(mqtt_server, 1883);
  mqttClient.setCallback(callback);
  if (mqttClient.connect(myRoom, mqtt_user, mqtt_password)) {
    mqttClient.publish(roomStatus,"hello world");
    
    // mqttClient.subscribe(setmaxtemp);
    // mqttClient.subscribe(setmintemp);
    // mqttClient.subscribe(setmaxhumidity);
    // mqttClient.subscribe(setminhumidity);

    Serial.print("mqtt connected : ");
    Serial.println(mqtt_server);
  }
}

void callback(char* topic, byte* payload, unsigned int length) {
  int i;

  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
}


void reconnect() {
  // Loop until we're reconnected
  

  while (!mqttClient.connected()) {
    Serial.print("Attempting MQTT connection...");
    
    // Attempt to connect
    if (mqttClient.connect(myRoom, mqtt_user, mqtt_password)) {
      Serial.print("connected : ");
      Serial.println(mqtt_server);
      


      // Once connected, publish an announcement...
      mqttClient.publish(roomStatus, "hello world");
      
    } else {
      Serial.print("failed, reconnecting state = ");
      Serial.print(mqttClient.state());
      Serial.print(" try : ");
      Serial.print(mqtt_reconnect+1);
      Serial.println(" try again in 2 seconds");
      // Wait 2 seconds before retrying
      delay(2000);
    }
    mqtt_reconnect++;
    if (mqtt_reconnect > MAXRETRY) {
      mqtt_reconnect = 0;
      break;
    }
  }
}
