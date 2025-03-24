#include <Arduino.h>
#include <WiFi.h>
#include "ESPExpress.h"  // Ensure you have the ESPExpress library installed

// Replace with your WiFi credentials
const char* ssid     = "your_SSID";
const char* password = "your_PASSWORD";

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

// Register WebSocket routes inline without using a separate header.
void registerWebSocketRoutes() {
  app.ws("/ws", [](uint8_t num, WStype_t type, uint8_t* payload, size_t length) {
    switch (type) {
      case WStype_CONNECTED:
        Serial.printf("WebSocket client #%u connected\n", num);
        break;
      case WStype_DISCONNECTED:
        Serial.printf("WebSocket client #%u disconnected\n", num);
        break;
      case WStype_TEXT: {
          String message = String((char*)payload).substring(0, length);
          Serial.printf("Received text from client #%u: %s\n", num, message.c_str());
          // Echo the message back to the client
          app.wsSendTXT(num, message);
        }
        break;
      default:
        break;
    }
  });
}

void setup() {
  Serial.begin(115200);
  connectToWiFi();
  
  // Register WebSocket route at /ws
  registerWebSocketRoutes();

  Serial.println("Starting WebSocket server on port 80");
  app.listen(); // This call starts the web server (blocking call if configured that way)
}

void loop() {
  // If app.listen() is non-blocking or for additional tasks,
  // continuously process WebSocket events.
  app.wsLoop();
}
