#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <config.h>
#include "EmonLib.h"      
#include <PubSubClient.h>  

EnergyMonitor emon1;        
unsigned long lastMeasurement = 0;

WiFiClient espClient;
PubSubClient client(espClient);

// mqtt recovery
void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(9600);
  pinMode(A0, INPUT);

  randomSeed(micros());

  //Initialize emon library
  emon1.current(A0, 80);  

  // Connect to Wi-Fi network with SSID and password
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.mode(WIFI_STA);
  WiFi.hostname("emon");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  
  // Print local IP address and start web server
  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  // mqtt
  client.setServer(mqtt_server, 1883);
}

char msg[50];

void loop() {
  // mqtt
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  unsigned long currentMillis = millis();  
  if(currentMillis - lastMeasurement > 1000) {    
    // emon
    double Irms = emon1.calcIrms(1480);
    // the power
    lastMeasurement = millis();
    snprintf(msg, 50, "{\"current\": %lf}", Irms);
    Serial.println(msg);
    client.publish("emon/current", msg);
  }
}

