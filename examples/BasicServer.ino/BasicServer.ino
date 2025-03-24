#include <Arduino.h>
#include <WiFi.h>
#include "ESPExpress.h"

// Replace with your WiFi credentials
const char* ssid     = "your_SSID";
const char* password = "your_PASSWORD";

// LED configuration
const int ledPin = 2;

// Create an instance of ESPExpress on port 80
ESPExpress app(80);

// Helper function to connect to WiFi
void connectToWiFi() {
  Serial.print("Connecting to WiFi");
  WiFi.begin(ssid, password);
  unsigned long startTime = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - startTime < 15000) {
    Serial.print(".");
    delay(500);
  }
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nConnected to WiFi");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("\nFailed to connect to WiFi");
  }
}

// Route handler to turn the LED on
void handleTurnOn(Request &req, Response &res) {
  digitalWrite(ledPin, HIGH);
  Serial.println("LED turned ON");
  res.send("LED turned ON");
}

// Route handler to turn the LED off
void handleTurnOff(Request &req, Response &res) {
  digitalWrite(ledPin, LOW);
  Serial.println("LED turned OFF");
  res.send("LED turned OFF");
}

void setup() {
  Serial.begin(115200);
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW);

  connectToWiFi();

  // Register the routes
  app.get("/manual/on", handleTurnOn);
  app.get("/manual/off", handleTurnOff);

  Serial.println("Starting server on port 80");
  app.listen(); // This call blocks the setup, starting the web server.
}

void loop() {
  // Nothing to do here since app.listen() is blocking.
}
