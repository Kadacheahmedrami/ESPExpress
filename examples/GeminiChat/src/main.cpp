#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include "ESPExpress.h"

// Replace with your WiFi credentials.
const char* ssid     = "your_SSID";
const char* password = "your_PASSWORD";

// Replace with your actual Gemini API token.
const char* Gemini_Token = "your_Gemini_Token";
const int maxTokens = 100;

// LED configuration (optional, for demonstration)
const int ledPin = 2;

// Create an instance of ESPExpress on port 80.
ESPExpress app(80);

// Helper function to connect to WiFi.
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

// Generate a strict prompt for Gemini that returns exactly one command.
String generatePrompt(const String& question) {
  return "You are a precise command interpreter for a digital LED. "
         "When given an input, respond with EXACTLY one of these commands: 'turn on', 'turn off', or 'no command'. "
         "Do not include any extra words, punctuation, or explanations. Input: " + question;
}

// Send a query to the Gemini API and return its command response.
String sendGeminiRequest(const String& question) {
  String command = "no command";
  HTTPClient http;
  String url = "https://generativelanguage.googleapis.com/v1beta/models/gemini-1.5-flash:generateContent?key=" + String(Gemini_Token);
  
  if (!http.begin(url)) {
    Serial.println("Failed to connect to Gemini API endpoint");
    return command;
  }
  
  http.addHeader("Content-Type", "application/json");
  String prompt = generatePrompt(question);
  String payload = "{\"contents\": [{\"parts\":[{\"text\":\"" + prompt + "\"}]}],"
                   "\"generationConfig\": {\"maxOutputTokens\": " + String(maxTokens) + "}}";
  
  int httpCode = http.POST(payload);
  if (httpCode == HTTP_CODE_OK) {
    String response = http.getString();
    StaticJsonDocument<1024> doc;
    DeserializationError error = deserializeJson(doc, response);
    if (!error) {
      command = doc["candidates"][0]["content"]["parts"][0]["text"].as<String>();
      command.trim();
      command.toLowerCase();
      Serial.println("Gemini returned: " + command);
    } else {
      Serial.println("JSON parse error: " + String(error.c_str()));
    }
  } else {
    Serial.println("HTTP POST failed: " + http.errorToString(httpCode));
  }
  
  http.end();
  return command;
}

// Process the Gemini command and control the LED accordingly.
void processCommand(const String& command) {
  Serial.println("Processing command: " + command);
  if (command == "turn on") {
    digitalWrite(ledPin, HIGH);
    Serial.println("LED turned ON");
  } else if (command == "turn off") {
    digitalWrite(ledPin, LOW);
    Serial.println("LED turned OFF");
  } else {
    Serial.println("No valid command received.");
  }
}

// Route handler for /api/ask – accepts a query parameter "q".
void handleApiAsk(Request &req, Response &res) {
  String query = req.getQuery("q");
  Serial.println("API query: " + query);
  
  // Assuming urlDecode() is provided by ESPExpress.
  String question = urlDecode(query);
  Serial.println("Decoded question: " + question);
  
  // Get the command response from Gemini.
  String command = sendGeminiRequest(question);
  processCommand(command);
  
  // Build a JSON response.
  String jsonResponse = "{\"answer\":\"" + command + "\"}";
  res.sendJson(jsonResponse);
}

void setup() {
  Serial.begin(115200);
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW);
  
  connectToWiFi();
  
  // Register the /api/ask route.
  app.get("/api/ask", handleApiAsk);
  
  Serial.println("Starting Gemini Chat server on port 80");
  app.listen(); // This starts the server and may block.
}

void loop() {
  // If app.listen() is non-blocking or for additional tasks, process events.
  app.wsLoop();
}
