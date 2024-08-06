/*
 * This section of the sketch program code is dedicated to set up of the code.
 * This means that here I am importing all necessary libraries and set all global variables.
 */
#include <Wire.h>
#include "SSD1306Wire.h"
#include <Arduino.h>
#include <ESP8266WebServer.h>
#include "secrets.h"
#include "HTMLPages.h"
#include <ESP8266WiFi.h>
#include <time.h>
#define TIMEZONE "BRT3"

#define HOSTNAME "esp8266webserver"
#define DISPLAY_ADDRESS 0x3c
#define SDA_PIN 14
#define SCL_PIN 12

SSD1306Wire display(DISPLAY_ADDRESS, SDA_PIN, SCL_PIN);
ESP8266WebServer server(80);

char deviceIP[50];
#include "wifiLogo.h"

// Function prototypes
void printToDisplay(const String &line1, const String &line2 = "", const String &line3 = "");
void connectToWiFi();
void drawWiFiLogo();
void handleRoot();

void setup() {
  Serial.begin(115200);
  delay(1000); // Wait for serial monitor to start completely

  display.init();
  display.flipScreenVertically();
  display.setFont(ArialMT_Plain_10);

  printToDisplay("Wellcome to ESP8266!", "Initializing the set up.", "It will be fast bro!");
  delay(2000);

  connectToWiFi();

  configTime(0, 0, "pool.ntp.org"); // Using external server to get the actual hour
  setenv("TZ", TIMEZONE, 1);
  /*
  Serial.println("\nGetting the current hour...");
  while (!time(nullptr)) {
    Serial.print(".");
    delay(1000);
  }
  Serial.println("\nReceived the current hour!");
  */
  tzset();

  WiFi.setHostname(HOSTNAME);

  server.on("/", handleRoot);
  server.begin();

  Serial.println("\nServer is alive!");
  printToDisplay("Server is alive!");
  delay(3000);
}

void printToDisplay(const String &line1, const String &line2, const String &line3) {
  display.clear();
  display.setFont(ArialMT_Plain_10);
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.drawString(0, 0, line1);
  if (line2 != "") display.drawString(0, 14, line2);
  if (line3 != "") display.drawString(0, 28, line3);
  display.display();
}

void drawWiFiLogo() {
  Serial.print("\nDrawing the WiFi logo...");
  display.clear();
  display.drawXbm(34, 15, WiFi_Logo_width, WiFi_Logo_height, WiFi_Logo_bits);
  display.display();
  delay(2000);
  Serial.print("\nDrew.");
}

void connectToWiFi() {
  Serial.printf("\nConnecting to %s", ssid);
  printToDisplay("Connecting...", String("SSID: ") + ssid);
  delay(1000);

  WiFi.begin(ssid, passPhrase);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nConnected!");
  IPAddress IP = WiFi.localIP();
  Serial.print("\nIP: ");
  Serial.println(IP);
  
  snprintf(deviceIP, sizeof(deviceIP), "IP: %s", IP.toString().c_str());
  printToDisplay("Connected!", deviceIP);
  
  drawWiFiLogo();
}

void handleRoot() {
  printToDisplay("Received a request!","Getting the current date...");
  delay(1000);

  time_t now = time(nullptr);
  struct tm timeinfo;
  localtime_r(&now, &timeinfo);

  char timeStr[90];
  snprintf(timeStr, sizeof(timeStr), "Brazilian local time (Brasília): %02d-%02d-%04d %02d:%02d:%02d (UTC-3)",
           timeinfo.tm_mday, timeinfo.tm_mon + 1, timeinfo.tm_year + 1900,
           timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);

  printToDisplay("Received the date!", "Creating HTTP response...");
  delay(1000);

  String html = "<!DOCTYPE html><html lang='pt-BR'><head><meta charset='UTF-8'><meta name='viewport' content='width=device-width, initial-scale=1.0'><title>Node MCU ESP8266 Webserver</title></head><body><p>";
  html += timeStr;
  html += "</p></body></html>";

  server.send(200, "text/html", html);

  printToDisplay("HTTP response sent!", "My work finish!");
  delay(2000);
}

void loop() {
  printToDisplay("Waiting for requests on", deviceIP);
  delay(500);
  server.handleClient();
}