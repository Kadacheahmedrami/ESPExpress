# ESPExpress

[![Build Status](https://img.shields.io/badge/build-passing-brightgreen)](https://github.com/kadacheahmedrami/ESPExpress)
[![License](https://img.shields.io/badge/License-MIT-blue.svg)](LICENSE)
[![PlatformIO Compatible](https://img.shields.io/badge/PlatformIO-compatible-brightgreen)](https://platformio.org)

**ESPExpress** is a lightweight, back-end C++ framework for ESP microcontrollers that mimics the popular Express.js API. Build robust web applications and APIs on your embedded devices with minimal overhead.

## ✨ Features

- **Express.js Inspired API:** Define routes for HTTP methods (GET, POST, PUT, DELETE, OPTIONS) using familiar syntax.
- **Middleware Support:** Chain middleware functions for logging, authentication, and request manipulation.
- **Static File Serving:** Serve files directly from SPIFFS/LittleFS with automatic content type detection.
- **Template Rendering:** Create dynamic HTML pages with simple variable substitution.
- **CORS Handling:** Built-in support for Cross-Origin Resource Sharing enables seamless API integrations.
- **WebSocket Integration:** Create real-time applications with WebSocket connection management.

## 🚀 Quick Start

### Installation

1. **Clone the Repository:**
   ```bash
   git clone https://github.com/kadacheahmedrami/ESPExpress.git
   ```

2. **Project Structure:**
   ```
   your_project/
   ├── data/                      # Static files for SPIFFS
   │   └── www/                   # Web content
   │       ├── index.html         # Main page
   │       ├── info.html          # Information page
   │       └── template.html      # Template for rendering
   │
   ├── lib/                       # Library code
   │   ├── ESPExpress/            # Core library files
   │   │   ├── ESPExpress.h       # Main library header
   │   │   └── ESPExpress.cpp     # Implementation
   │   └── routes/                # Route modules
   │       ├── routeExamples/
   │       │   ├── routeExamples.h
   │       │   └── routeExamples.cpp
   │       └── websocket/
   │           ├── websocket.h
   │           └── websocket.cpp
   └── src/                       # Demo application
       └── main.cpp               # Example usage
   ```

3. **Upload Static Files to Flash:**
   ```bash
   pio run -t uploadfs
   ```
   This uploads your HTML files and other static content to the ESP flash memory.

4. **Upload Your Application:**
   ```bash
   pio run -t upload
   ```

### Example Code

```cpp
#include <WiFi.h>
#include <SPIFFS.h>
#include "ESPExpress.h"
#include "websocket/websocket.h"          // Import WebSocket routes
#include "routeExamples/routeExamples.h"  // Import HTTP route examples

const char* ssid = "Your_SSID";         // WiFi SSID
const char* password = "Your_PASSWORD"; // WiFi password

ESPExpress app(80);                   // Create server instance on port 80

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  WiFi.begin(ssid, password);         // Connect to WiFi
  Serial.println("Connecting to WiFi...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.print("Connected! IP address: "); 
  Serial.println(WiFi.localIP());

  if (!SPIFFS.begin(true)) {           // Mount SPIFFS
    Serial.println("Failed to mount SPIFFS");
    return;
  }
  
  app.use([](Request &req, Response &res, std::function<void()> next) { // Log request path
    Serial.print("Request received for: "); 
    Serial.println(req.path); 
    next();
  });
  
  app.enableCORS("*");                // Enable CORS for all origins
  
  registerRouteExamples(app);         // Register HTTP route examples
  registerWebSocketRoutes(app);       // Register WebSocket routes
  
  Serial.println("Starting ESPExpress server...");
  app.listen("Server is running..."); // Start the server
}

void loop() {
  app.wsLoop();                       // Process WebSocket events
}
```

### Route Examples Implementation

Here's how routes are configured in the `routeExamples.cpp` file:

```cpp
#include "routeExamples.h"

void registerRouteExamples(ESPExpress &app) {
  // Serve the index.html file on the root "/"
  app.get("/", [](Request &req, Response &res) {
    res.sendFile("/www/index.html");
  });

  // GET route with a dynamic parameter: /user/:id
  app.get("/user/:id", [](Request &req, Response &res) {
    String userId = req.getParam("id");
    res.send("<h1>User Profile</h1><p>User ID: " + userId + "</p>");
  });

  // POST route to test receiving a body payload.
  app.post("/data", [](Request &req, Response &res) {
    res.send("<h1>POST Data Received</h1><p>Body: " + req.body + "</p>");
  });

  // PUT route example.
  app.put("/update", [](Request &req, Response &res) {
    res.send("<h1>PUT Update</h1><p>Update data: " + req.body + "</p>");
  });

  // DELETE route example.
  app.del("/delete", [](Request &req, Response &res) {
    res.send("<h1>DELETE Request</h1><p>Resource deleted.</p>");
  });

  // OPTIONS route example (useful for CORS preflight requests).
  app.options("/test", [](Request &req, Response &res) {
    res.status(204).end();  // No Content response
  });

  // Template Rendering: GET /template
  // Capture app by reference so that we can call app.render() inside the lambda.
  app.get("/template", [&app](Request &req, Response &res) {
    std::map<String, String> vars;
    vars["title"] = "ESPExpress Template";
    vars["message"] = "Hello from a rendered template!";
    app.render(res, "/www/template.html", vars);
  });

  // Static File Serving: Map URL path "/static" to SPIFFS folder "/www"
  app.serveStatic("/static", "/www");
}
```

## 📚 Documentation

### Routing System

- **HTTP Routes:** Register routes using `app.get()`, `app.post()`, etc. Each route processes requests and sends responses.
- **Middleware:** Use `app.use()` to chain middleware functions for logging, authentication, or request modification.
- **Static Files & Templates:** Serve files with `app.serveStatic()` and generate dynamic HTML with `app.render()`.
- **Route Parameters:** Access dynamic route parameters with `req.getParam()` (e.g., `/user/:id`).

### WebSocket Integration

Register WebSocket routes with `app.ws()` to handle connections, messages, and disconnections for real-time communication.

### CORS Support

Enable CORS with `app.enableCORS("*")` to allow cross-origin requests from any domain.

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

Happy coding with **ESPExpress**! Build amazing back-end applications on your ESP microcontrollers with ease.
