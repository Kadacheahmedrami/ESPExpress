# ESPExpress - Lightweight Web Server Library for ESP32

ESPExpress is a **lightweight** and **flexible** web server library for ESP32, enabling developers to create efficient web-based applications with minimal effort. It provides an intuitive API for handling HTTP requests, serving static files, managing WebSockets, and enabling CORS support.

## 🚀 Features

- **HTTP Request Handling**: Support for `GET`, `POST`, `PUT`, `DELETE`, and `OPTIONS` requests.
- **Middleware Support**: Easily add global middleware functions.
- **Static File Serving**: Serve files from SPIFFS or other storage.
- **WebSocket Support**: Handle real-time communication with WebSockets.
- **CORS Support**: Enable Cross-Origin Resource Sharing.
- **Template Rendering**: Serve dynamic HTML with variable replacements.

## 📌 Installation

Include ESPExpress in your project:

```cpp
#include "ESPExpress.h"
```

## 📖 Usage Example

This example demonstrates how to create an ESP32 web server that controls an LED:

```cpp
#include <Arduino.h>
#include <WiFi.h>
#include "ESPExpress.h"

const char* ssid     = "your_SSID";
const char* password = "your_PASSWORD";
const int ledPin = 2;
ESPExpress app(80);

void connectToWiFi() {
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }
}

void handleTurnOn(Request &req, Response &res) {
  digitalWrite(ledPin, HIGH);
  res.send("LED turned ON");
}

void handleTurnOff(Request &req, Response &res) {
  digitalWrite(ledPin, LOW);
  res.send("LED turned OFF");
}

void setup() {
  Serial.begin(115200);
  pinMode(ledPin, OUTPUT);
  connectToWiFi();
  
  app.get("/on", handleTurnOn);
  app.get("/off", handleTurnOff);
  
  app.listen();
}

void loop() {}
```

## 🔗 WebSocket Support

Enable real-time WebSocket communication with:

```cpp
app.ws("/ws", [](uint8_t num, WStype_t type, uint8_t* payload, size_t length) {
  if (type == WStype_TEXT) {
    Serial.printf("Client %d sent: %s\n", num, payload);
  }
});
```

## 🌍 CORS Support

Enable CORS to allow cross-origin requests:

```cpp
app.enableCORS("*");
```

## 🤝 Contributing

Contributions are welcome! Follow these steps:

1. **Fork the Repository**
2. **Create a new branch:**
   ```bash
   git checkout -b feature/YourFeature
   ```
3. **Commit your changes:**
   ```bash
   git commit -am 'Add new feature'
   ```
4. **Push to your branch:**
   ```bash
   git push origin feature/YourFeature
   ```
5. **Open a Pull Request** on GitHub.

See our [CONTRIBUTING.md](https://github.com/kadacheahmedrami/ESPExpress/blob/main/CONTRIBUTING.md) for details.

## 📝 License

This project is licensed under the [MIT License](LICENSE).

## 📞 Contact

For support or questions:
- [Open an issue](https://github.com/kadacheahmedrami/ESPExpress/issues)
- Email: [a_kadache@estin.dz](mailto:a_kadache@estin.dz)

---

Happy coding with **ESPExpress**! Build efficient web-based applications on your ESP32 effortlessly. 🚀

