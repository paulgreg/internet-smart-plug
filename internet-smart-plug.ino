#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <Ticker.h>

// parameters.h file contains setup parameters like wifi ssid and password, host to connect, etc
#include "parameters.h"

unsigned const int HTTPS_PORT = 443;

Ticker sleepTicker;

void setup() {
  pinMode(16, WAKEUP_PULLUP);
  Serial.begin(115200);
  delay(1000);
  Serial.println("");
  Serial.println("Internet Smart Plug");
  sleepTicker.once_ms(WATCHDOG_SLEEP_TIMEOUT, &sleep); // maximum time allowed for loop function to run (watchdog)
  delay(1000);
  connectToWifi();
}

void loop() {
  boolean online;
  unsigned short i = 0;

  do {
    boolean check1 = get(CHECK_HOST1, CHECK_URL1);
    boolean check2 = get(CHECK_HOST2, CHECK_URL2);
    online = (check1 || check2);
  } while(!online && i++ < MAX_TRIES);

  Serial.println("");
  Serial.print("status: ");
  Serial.println(online ? "OK" : "KO");
  Serial.println("");

  get(MONITOR_HOST, online ? MONITOR_URL_UP : MONITOR_URL_DOWN);

  if (!online) {
    cyclePower();
  } else {
    Serial.println("nothing to do");
  }

  sleep();
}

void sleep() {
  Serial.print("going to sleep: ");
  Serial.println(SLEEP_TIME);
  ESP.deepSleep(SLEEP_TIME, WAKE_RF_DISABLED); // When it wakes up we start again in setup().
  delay(5000); // It can take a while for the ESP to actually go to sleep.
}

// Turn plug off then on
void cyclePower() {
  Serial.println("turning plug off");
  // TODO: GPIO -> HIGH
  delay(2500);
  // TODO: GPIO -> LOW
  Serial.println("turning plug on");
}

void connectToWifi() {
  unsigned int i = 0;

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

boolean get(const char* host, const char* url) {
  WiFiClientSecure httpsClient;
  boolean status = false;
  Serial.print(">  ");
  Serial.print(host);
  Serial.print(":");
  Serial.print(HTTPS_PORT);

  if (!httpsClient.connect(host, HTTPS_PORT)) {
    Serial.println("");
    Serial.println("connection failed");
    return false;
  }

  Serial.println(url);

  httpsClient.print(String("GET ") + url + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" +
               "User-Agent: ESP8266\r\n" +
               "Connection: close\r\n\r\n");

  unsigned long timeout = millis();
  while (httpsClient.available() == 0) {
    delay(1);
    if (millis() - timeout > REQUEST_TIMEOUT) {
      Serial.println("client Timeout !");
      httpsClient.stop();
      return false;
    }
  }

  if (httpsClient.connected()) {
    String line = httpsClient.readStringUntil('\n');
    Serial.print("< ");
    Serial.println(line);
    status = true;
  }
  return status;
}



