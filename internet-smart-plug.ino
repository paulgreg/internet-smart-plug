#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>

// parameters.h file contains setup parameters like wifi ssid and password, host to connect, etc
#include "parameters.h"

void setup() {
  Serial.begin(115200);
  Serial.println("");
  Serial.println("Internet Smart Plug");
}

void loop() {
  if (WiFi.SSID() != WIFI_SSID) {
    Serial.print("connecting to ");
    Serial.println(WIFI_SSID);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  }
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("wifi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  boolean check1 = get(HTTP_CHECK_HOST1, HTTP_CHECK_URL1);
  boolean check2 = get(HTTP_CHECK_HOST2, HTTP_CHECK_URL2);
  boolean online = (check1 || check2);

  Serial.print("Status:");
  Serial.println(online);
  
  getSecure(HTTPS_MONITOR_HOST, online ? HTTPS_MONITOR_URL_UP : HTTPS_MONITOR_URL_DOWN);

  Serial.println("disconnecting wifi");
  WiFi.disconnect();

  Serial.print("waiting: ");
  Serial.println(SLEEP_TIME);
  delay(SLEEP_TIME);
}


boolean getSecure(const char* host, const char* url) {
  WiFiClientSecure client;

  Serial.print("connecting to ");
  Serial.print(host);
  Serial.println(":443");

  if (!client.connect(host, 443)) {
    Serial.println("connection failed");
    return false;
  }

  Serial.print("requesting ");
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

  Serial.println("request sent");
  if (client.connected()) {
    String line = client.readStringUntil('\n');
    Serial.println(line);
  }
  Serial.println("closing connection");
  return true;
}

boolean get(const char* host, const char* url) {
  WiFiClient client;

  Serial.print("connecting to ");
  Serial.print(host);
  Serial.println(":80");

  if (!client.connect(host, 80)) {
    Serial.println("connection failed");
    return false;
  }

  Serial.print("requesting ");
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

  Serial.println("request sent");
  if (client.connected()) {
    String line = client.readStringUntil('\n');
    Serial.println(line);
  }
  Serial.println("closing connection");
  return true;
}

