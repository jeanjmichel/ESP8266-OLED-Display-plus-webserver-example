/*
 * This section of the sketch program code is dedicated to set up of the code.
 * This means that here I am importing all necessary libraries and set all global variables.
 */
#include <Wire.h>
#include "SSD1306Wire.h"
#include <Arduino.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include "secrets.h"
#include <ESP8266WiFi.h>
#include <time.h>
#include "wifiLogo.h"

#define TIMEZONE "BRT3"
#define HOSTNAME "esp8266webserver"
#define DISPLAY_ADDRESS 0x3c
#define SDA_PIN 14
#define SCL_PIN 12

SSD1306Wire display(DISPLAY_ADDRESS, SDA_PIN, SCL_PIN);
ESP8266WebServer server(80);

char deviceIP[50];
char deviceURL[50];

/*
 * This section prototypes all the utilized functions, essentially outlining their 'interface'
 * or contract, which will be implemented further down in the code.
 */
void printToDisplay(const String &line1, const String &line2 = "", const String &line3 = "");
void connectToWiFi();
void drawWiFiLogo();
void handleRoot();

/*
 * The setup method is performed only once during the device's initialization to prepare it for use. 
 */
void setup() {
  delay(3000); //The pause ensures that the serial monitor has time to initialize properly.
  Serial.begin(115200);
  
  display.init();
  display.flipScreenVertically();
  display.setFont(ArialMT_Plain_10);

  printToDisplay("Wellcome to ESP8266!", "Initializing the setup.", "It will be fast dude!");
  delay(3000);

  connectToWiFi();

  configTime(0, 0, "pool.ntp.org"); // Using external server to get the actual date
  setenv("TZ", TIMEZONE, 1); //Adjust the time zone, in this case to Brazilian time.
  tzset();

  server.on("/", HTTP_GET, handleRoot); //This handler will manage all HTTP requests sent to the root (/) endpoint.
  server.enableCORS(true);
  server.onNotFound([]() { //When a request is made for a page that doesn't exist, this is how it's managed.
    Serial.println("\nPage not found :(");
    printToDisplay("Page not found :(");
    server.send(404, "text/html", "Page not found :(");
    delay(1000);
  });

  Serial.println("\nSet up finished!");
  printToDisplay("Set up finished!");
  delay(1000); //Time delay after displaying the message on screen.
}

/*
 * This function centralizes the method of presenting information on the display. 
 * It allows for three lines of text to be passed as parameters and then visually renders them on the screen.
 */
void printToDisplay(const String &line1, const String &line2, const String &line3) {
  display.clear();
  display.setFont(ArialMT_Plain_10);
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.drawString(0, 0, line1);
  if (line2 != "") display.drawString(0, 14, line2);
  if (line3 != "") display.drawString(0, 28, line3);
  display.display();
}

/*
 * This function shows the WiFi symbol on the screen.
 */
void drawWiFiLogo() {
  Serial.print("\nDrawing the WiFi logo...");
  display.clear();
  display.drawXbm(34, 15, WiFi_Logo_width, WiFi_Logo_height, WiFi_Logo_bits);
  display.display();
  delay(2000);
  Serial.print("\nDrew.");
}

/*
 * This function connects the device to a WiFi network.
 */
void connectToWiFi() {
  Serial.printf("\nConnecting to %s", ssid);
  printToDisplay("Connecting...", String("SSID: ") + ssid);
  delay(1000);

  WiFi.begin(ssid, passPhrase);
  WiFi.setHostname(HOSTNAME);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  server.begin();
  Serial.println("\nConnected!");

  if (MDNS.begin(HOSTNAME)) { //Configures the device's host name to respond to inquiries such as http://hostname.
    Serial.println("MDNS responder started");
  }
  else {
    Serial.println("Error setting up MDNS responder!");
  }

  snprintf(deviceURL, sizeof(deviceURL), "http://%s", HOSTNAME);
  IPAddress IP = WiFi.localIP();
  Serial.print("\nIP: ");
  Serial.println(IP);  
  snprintf(deviceIP, sizeof(deviceIP), "IP: %s", IP.toString().c_str());
  printToDisplay("Connected!", deviceIP, deviceURL);
  
  drawWiFiLogo();
}

/*
 * The function responsible for executing tasks when a request arrives at the root endpoint (/).
 */
void handleRoot() {
  printToDisplay("Received a request!","Getting the current date...");
  Serial.println("\nReceived a request! Getting the current date...");
  delay(1000);

  time_t now = time(nullptr);
  struct tm timeinfo;
  localtime_r(&now, &timeinfo);

  char timeStr[90];
  snprintf(timeStr, sizeof(timeStr), "Brazilian local time (Brasília): %02d-%02d-%04d %02d:%02d:%02d (UTC-3)",
           timeinfo.tm_mday, timeinfo.tm_mon + 1, timeinfo.tm_year + 1900,
           timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);

  printToDisplay("Data is ok!", "Creating HTTP response...");
  Serial.println("Data is ok! Creating HTTP response...");

  delay(1000);

  String html = "<!DOCTYPE html><html lang='pt-BR'><head><meta charset='UTF-8'><meta name='viewport' content='width=device-width, initial-scale=1.0'><title>Node MCU ESP8266 Webserver</title></head><body><p>";
  html += timeStr;
  html += "</p></body></html>";

  printToDisplay("HTTP response sent!", "My work finish!");
  Serial.println("HTTP response sent! My work finish!");
  server.sendHeader("Cache-Control", "no-cache");
  server.send(200, "text/html", html);
}

/*
 * When the device is powered on, this code executes.
 */
void loop() {
  printToDisplay("Waiting for requests on", deviceIP, deviceURL);
  server.handleClient();
  MDNS.update();
}