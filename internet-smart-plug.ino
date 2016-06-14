#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>

// parameters.h file contains setup parameters like wifi ssid and password, host to connect, etc
#include "parameters.h"

void setup() {
  Serial.begin(115200);
  Serial.println("");
  Serial.println("Internet Smart Plug");
  delay(1000);
}

void loop() {
  boolean online;
  unsigned short i = 0;
  WiFiClient httpClient;
  WiFiClientSecure httpsClient;

  do {
    connectToWifi();
    
    boolean check1 = get(httpClient, 80, CHECK_HOST1, CHECK_URL1);
    boolean check2 = get(httpClient, 80, CHECK_HOST2, CHECK_URL2);
    
    online = (check1 || check2);
    
  } while(!online && i++ < MAX_TRIES);

  Serial.println("");
  Serial.print("status: ");
  Serial.println(online ? "OK" : "KO");
  Serial.println("");

  get(httpsClient, 443, MONITOR_HOST, online ? MONITOR_URL_UP : MONITOR_URL_DOWN);

  if (!online) {
    cyclePower();
  } else {
    Serial.println("nothing to do");
  }

  Serial.println("");
  Serial.println("disconnecting wifi");
  WiFi.disconnect();

  Serial.println("");
  Serial.print("waiting: ");
  Serial.println(SLEEP_TIME);
  delay(SLEEP_TIME);
}

void cyclePower() {
  // Turn plug off then on
  Serial.println("turning plug off");
  // TODO: GPIO -> HIGH
  delay(2500);
  // TODO: GPIO -> LOW
  Serial.println("turning plug on");
}

void connectToWifi() {
    unsigned int i = 0;
    
    if (WiFi.SSID() != WIFI_SSID) {
      Serial.println("");
      Serial.print("connecting to ");
      Serial.println(WIFI_SSID);
      WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
      while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
        i++;
        if (i > 50) {
          // Maybe cyclePower() ?
          ESP.restart();
        }
      }
      Serial.println("");
      Serial.println("wifi connected");
      Serial.print("IP address: ");
      Serial.println(WiFi.localIP());
      Serial.println("");
    }
}

boolean get(WiFiClient &client, unsigned int port, const char* host, const char* url) {
  Serial.print(">  ");
  Serial.print(host);
  Serial.print(":");
  Serial.print(port);

  if (!client.connect(host, port)) {
    Serial.println("");
    Serial.println("connection failed");
    return false;
  }

  Serial.println(url);

  client.print(String("GET ") + url + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" +
               "User-Agent: ESP8266\r\n" +
               "Connection: close\r\n\r\n");

  unsigned long timeout = millis();
  while (client.available() == 0) {
    if (millis() - timeout > REQUEST_TIMEOUT) {
      Serial.println("client Timeout !");
      client.stop();
      return false;
    }
  }

  if (client.connected()) {
    String line = client.readStringUntil('\n');
    Serial.print("< ");
    Serial.println(line);
  }
  return true;
}

