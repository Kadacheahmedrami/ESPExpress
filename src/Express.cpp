#include "ESPExpress.h"

// -------------------------
// Utility Functions
// -------------------------

// Convert HTTP method string to enum.
HttpMethod stringToMethod(const String& method) {
  if (method == "GET") return GET;
  if (method == "POST") return POST;
  if (method == "PUT") return PUT;
  if (method == "DELETE") return DELETE;
  if (method == "OPTIONS") return OPTIONS;
  return GET; // default to GET if unknown
}

// Convert HTTP method enum to string.
String methodToString(HttpMethod method) {
  switch (method) {
    case GET: return "GET";
    case POST: return "POST";
    case PUT: return "PUT";
    case DELETE: return "DELETE";
    case OPTIONS: return "OPTIONS";
    default: return "UNKNOWN";
  }
}

// URL decode a percent-encoded string.
String urlDecode(const String& text) {
  String decoded = "";
  char temp[3] = {0, 0, 0};
  for (size_t i = 0; i < text.length(); i++) {
    char c = text[i];
    if (c == '+') {
      decoded += ' ';
    } else if (c == '%' && i + 2 < text.length()) {
      temp[0] = text[i + 1];
      temp[1] = text[i + 2];
      decoded += char(strtol(temp, NULL, 16));
      i += 2;
    } else {
      decoded += c;
    }
  }
  return decoded;
}

// Return content type based on file extension.
String getContentType(const String& path) {
  if (path.endsWith(".html")) return "text/html";
  if (path.endsWith(".css")) return "text/css";
  if (path.endsWith(".js")) return "application/javascript";
  if (path.endsWith(".json")) return "application/json";
  if (path.endsWith(".png")) return "image/png";
  if (path.endsWith(".jpg") || path.endsWith(".jpeg")) return "image/jpeg";
  if (path.endsWith(".gif")) return "image/gif";
  if (path.endsWith(".ico")) return "image/x-icon";
  if (path.endsWith(".xml")) return "text/xml";
  if (path.endsWith(".pdf")) return "application/pdf";
  if (path.endsWith(".zip")) return "application/zip";
  if (path.endsWith(".gz")) return "application/gzip";
  if (path.endsWith(".svg")) return "image/svg+xml";
  return "text/plain";
}

// Match a route pattern (which may include parameters) to a URL path.
bool pathMatch(const String& pattern, const String& path, std::map<String, String>& params) {
  // Direct match
  if (pattern == path) return true;
  
  int pIdx = 0, pathIdx = 0;
  while (pIdx < pattern.length() && pathIdx < path.length()) {
    if (pattern[pIdx] == ':') {
      // Parameter name extraction.
      int paramStart = pIdx + 1;
      while (pIdx < pattern.length() && pattern[pIdx] != '/') pIdx++;
      String paramName = pattern.substring(paramStart, pIdx);
      
      // Extract parameter value from the URL.
      int valueStart = pathIdx;
      while (pathIdx < path.length() && path[pathIdx] != '/') pathIdx++;
      String paramValue = path.substring(valueStart, pathIdx);
      
      // Decode the parameter and store it.
      params[paramName] = urlDecode(paramValue);
    } else {
      if (pattern[pIdx] != path[pathIdx]) return false;
      pIdx++;
      pathIdx++;
    }
  }
  
  // If both pattern and path are fully processed, we have a match.
  return (pIdx == pattern.length() && pathIdx == path.length());
}

// -------------------------
// Request Implementation
// -------------------------

// The constructor extracts the request line parts.
Request::Request(const String& requestLine) {
  // Expected format: METHOD <path> <version>
  int firstSpace = requestLine.indexOf(' ');
  int secondSpace = requestLine.indexOf(' ', firstSpace + 1);
  
  if (firstSpace != -1 && secondSpace != -1) {
    String methodStr = requestLine.substring(0, firstSpace);
    method = stringToMethod(methodStr);
    
    // Get the full path which may include query string.
    String fullPath = requestLine.substring(firstSpace + 1, secondSpace);
    version = requestLine.substring(secondSpace + 1);
    
    // Set the path; then let parseQuery() extract any query string.
    path = fullPath;
    parseQuery();
  }
}

void Request::addHeader(const String& line) {
  int colonPos = line.indexOf(':');
  if (colonPos > 0) {
    String name = line.substring(0, colonPos);
    String value = line.substring(colonPos + 1);
    value.trim();
    headers[name] = value;
  }
}

// Check if path contains a query string; if so, split it out.
void Request::parseQuery() {
  int queryPos = path.indexOf('?');
  if (queryPos != -1) {
    // Extract query substring and update path.
    String queryString = path.substring(queryPos + 1);
    path = path.substring(0, queryPos);
    
    // Split query string by '&'
    int start = 0;
    while (start < queryString.length()) {
      int ampPos = queryString.indexOf('&', start);
      if (ampPos == -1) ampPos = queryString.length();
      
      int equalsPos = queryString.indexOf('=', start);
      if (equalsPos != -1 && equalsPos < ampPos) {
        String key = queryString.substring(start, equalsPos);
        String value = queryString.substring(equalsPos + 1, ampPos);
        query[urlDecode(key)] = urlDecode(value);
      }
      start = ampPos + 1;
    }
  }
}

String Request::getParam(const String& name) const {
  auto it = params.find(name);
  return (it != params.end()) ? it->second : "";
}

String Request::getHeader(const String& name) const {
  auto it = headers.find(name);
  return (it != headers.end()) ? it->second : "";
}

String Request::getQuery(const String& name) const {
  auto it = query.find(name);
  return (it != query.end()) ? it->second : "";
}

// -------------------------
// Response Implementation
// -------------------------

Response::Response(WiFiClient& client) : client(client), headersSent(false), statusCode(200) {
  headers["Content-Type"] = "text/html";
  headers["Connection"] = "close";
}

Response& Response::status(int code) {
  statusCode = code;
  return *this;
}

Response& Response::setHeader(const String& name, const String& value) {
  headers[name] = value;
  return *this;
}

// Send headers if they haven't been sent already.
void Response::sendHeaders() {
  if (!headersSent) {
    client.print("HTTP/1.1 ");
    client.print(statusCode);
    // Simple status text; ideally, you might use a lookup table.
    switch (statusCode) {
      case 200: client.println(" OK"); break;
      case 201: client.println(" Created"); break;
      case 204: client.println(" No Content"); break;
      case 400: client.println(" Bad Request"); break;
      case 401: client.println(" Unauthorized"); break;
      case 403: client.println(" Forbidden"); break;
      case 404: client.println(" Not Found"); break;
      case 500: client.println(" Internal Server Error"); break;
      default: client.println(" Unknown"); break;
    }
    
    for (auto const &header : headers) {
      client.print(header.first);
      client.print(": ");
      client.println(header.second);
    }
    client.println();
    headersSent = true;
  }
}

Response& Response::send(const String& body) {
  sendHeaders();
  client.print(body);
  return *this;
}

Response& Response::sendJson(const String& json) {
  headers["Content-Type"] = "application/json";
  return send(json);
}

Response& Response::sendFile(const String& path) {
  if (SPIFFS.exists(path)) {
    File file = SPIFFS.open(path, "r");
    if (file) {
      headers["Content-Type"] = getContentType(path);
      sendHeaders();
      const size_t bufferSize = 1024;
      uint8_t buffer[bufferSize];
      while (file.available()) {
        size_t bytesRead = file.read(buffer, bufferSize);
        client.write(buffer, bytesRead);
      }
      file.close();
    } else {
      status(500).send("Failed to open file");
    }
  } else {
    status(404).send("File not found");
  }
  return *this;
}

void Response::end() {
  if (!headersSent) {
    sendHeaders();
  }
  client.stop();
}

// -------------------------
// Route Matching
// -------------------------

bool Route::matches(HttpMethod m, const String& reqPath, std::map<String, String>& outParams) const {
  if (m != method) return false;
  return pathMatch(pattern, reqPath, outParams);
}

// -------------------------
// WebSocket Implementation
// -------------------------

// Callback adapter to handle WebSocket events
void webSocketEventHandler(uint8_t num, WStype_t type, uint8_t* payload, size_t length) {
  // This function will be replaced with a lambda that calls the instance method
}

// -------------------------
// ESPExpress Implementation
// -------------------------

ESPExpress::ESPExpress(uint16_t port)
  : _server(port), _corsEnabled(false), _corsOrigin("*"), 
    _wsEnabled(false), _webSocketServer(nullptr),
    _wsConnectCallback(nullptr), _wsDisconnectCallback(nullptr), 
    _wsMessageCallback(nullptr), _wsEventCallback(nullptr) {
  // For ESP32, initialize SPIFFS if using it.
  if (!SPIFFS.begin(true)) {
    Serial.println("SPIFFS Mount Failed");
  }
}

ESPExpress::~ESPExpress() {
  _server.stop();
  if (_webSocketServer != nullptr) {
    delete _webSocketServer;
  }
}

ESPExpress& ESPExpress::get(const String& path, HandlerFunction handler) {
  Route route;
  route.method = GET;
  route.pattern = path;
  route.handler = handler;
  
  // Extract parameter names from route (e.g., /user/:id)
  int start = 0;
  while (true) {
    int colonPos = path.indexOf(':', start);
    if (colonPos == -1) break;
    int endPos = path.indexOf('/', colonPos);
    if (endPos == -1) endPos = path.length();
    route.paramNames.push_back(path.substring(colonPos + 1, endPos));
    start = endPos;
  }
  _routes.push_back(route);
  return *this;
}

ESPExpress& ESPExpress::post(const String& path, HandlerFunction handler) {
  Route route;
  route.method = POST;
  route.pattern = path;
  route.handler = handler;
  _routes.push_back(route);
  return *this;
}

ESPExpress& ESPExpress::put(const String& path, HandlerFunction handler) {
  Route route;
  route.method = PUT;
  route.pattern = path;
  route.handler = handler;
  _routes.push_back(route);
  return *this;
}

ESPExpress& ESPExpress::del(const String& path, HandlerFunction handler) {
  Route route;
  route.method = DELETE;
  route.pattern = path;
  route.handler = handler;
  _routes.push_back(route);
  return *this;
}

ESPExpress& ESPExpress::options(const String& path, HandlerFunction handler) {
  Route route;
  route.method = OPTIONS;
  route.pattern = path;
  route.handler = handler;
  _routes.push_back(route);
  return *this;
}

ESPExpress& ESPExpress::all(const String& path, HandlerFunction handler) {
  get(path, handler);
  post(path, handler);
  put(path, handler);
  del(path, handler);
  options(path, handler);
  return *this;
}

ESPExpress& ESPExpress::use(MiddlewareFunction middleware) {
  _middleware.push_back(middleware);
  return *this;
}

ESPExpress& ESPExpress::serveStatic(const String& virtualPath, const String& fsPath) {
  _staticPaths[virtualPath] = fsPath;
  return *this;
}

ESPExpress& ESPExpress::enableCORS(const String& origin) {
  _corsEnabled = true;
  _corsOrigin = origin;
  return *this;
}

// WebSocket implementation
ESPExpress& ESPExpress::ws(const String& path, WebSocketEventCallback callback) {
  _wsPath = path;
  _wsEventCallback = callback;
  _wsEnabled = true;
  
  // Create WebSocket server on port 81 (standard WebSocket port)
  if (_webSocketServer == nullptr) {
    _webSocketServer = new WebSocketsServer(81);
    
    // Set up WebSocket server callback
    _webSocketServer->onEvent([this](uint8_t num, WStype_t type, uint8_t* payload, size_t length) {
      this->handleWebSocketEvent(num, type, payload, length);
    });
    
    // Start WebSocket server
    _webSocketServer->begin();
  }
  
  return *this;
}

void ESPExpress::handleWebSocketEvent(uint8_t num, WStype_t type, uint8_t* payload, size_t length) {
  // If user provided a callback for all events, call it
  if (_wsEventCallback) {
    _wsEventCallback(num, type, payload, length);
    return;
  }
  
  // Otherwise handle specific event types
  switch (type) {
    case WStype_CONNECTED:
      if (_wsConnectCallback) _wsConnectCallback(num);
      break;
      
    case WStype_DISCONNECTED:
      if (_wsDisconnectCallback) _wsDisconnectCallback(num);
      break;
      
    case WStype_TEXT:
    case WStype_BIN:
      if (_wsMessageCallback) _wsMessageCallback(num, payload, length);
      break;
      
    default:
      // Other event types not handled specifically
      break;
  }
}

ESPExpress& ESPExpress::onWsConnect(WebSocketClientCallback callback) {
  _wsConnectCallback = callback;
  return *this;
}

ESPExpress& ESPExpress::onWsDisconnect(WebSocketClientCallback callback) {
  _wsDisconnectCallback = callback;
  return *this;
}

ESPExpress& ESPExpress::onWsMessage(WebSocketMessageCallback callback) {
  _wsMessageCallback = callback;
  return *this;
}

void ESPExpress::wsBroadcast(uint8_t* payload, size_t length) {
  if (_wsEnabled && _webSocketServer != nullptr) {
    _webSocketServer->broadcastBIN(payload, length);
  }
}

void ESPExpress::wsBroadcastTXT(const String& text) {
  if (_wsEnabled && _webSocketServer != nullptr) {
    String mutableText = text;  // Create a mutable copy
    _webSocketServer->broadcastTXT(mutableText);
  }
}


void ESPExpress::wsSend(uint8_t num, uint8_t* payload, size_t length) {
  if (_wsEnabled && _webSocketServer != nullptr) {
    _webSocketServer->sendBIN(num, payload, length);
  }
}

void ESPExpress::wsSendTXT(uint8_t num, const String& text) {
  if (_wsEnabled && _webSocketServer != nullptr) {
    String mutableText = text;  // Create a mutable copy
    _webSocketServer->sendTXT(num, mutableText);
  }
}


void ESPExpress::wsLoop() {
  if (_wsEnabled && _webSocketServer != nullptr) {
    _webSocketServer->loop();
  }
}

void ESPExpress::listen(const char* message) {
  _server.begin();
  if (message) {
    Serial.println(message);
  } else {
    Serial.println("ESPExpress server started");
  }
  
  if (_wsEnabled && _webSocketServer != nullptr) {
    Serial.println("WebSocket server started on path: " + _wsPath);
  }
  
  while (true) {
    WiFiClient client = _server.available();
    if (client) {
      processClient(client);
    }
    
    // Handle WebSocket events if enabled
    if (_wsEnabled && _webSocketServer != nullptr) {
      _webSocketServer->loop();
    }
    
    delay(1);
  }
}

void ESPExpress::processClient(WiFiClient& client) {
  unsigned long timeout = millis() + 5000; // 5-second timeout
  while (client.connected() && !client.available() && millis() < timeout) {
    delay(1);
  }
  if (!client.available()) {
    client.stop();
    return;
  }
  
  // Read request line.
  String requestLine = client.readStringUntil('\r');
  client.readStringUntil('\n'); // consume LF
  
  Request req(requestLine);
  
  // Read headers.
  while (client.connected()) {
    String line = client.readStringUntil('\r');
    client.readStringUntil('\n');
    if (line.length() == 0) break;
    req.addHeader(line);
  }
  
  // Read body if Content-Length header exists.
  if (req.headers.find("Content-Length") != req.headers.end()) {
    int contentLength = req.headers["Content-Length"].toInt();
    char* bodyBuffer = new char[contentLength + 1];
    int bytesRead = 0;
    while (bytesRead < contentLength && client.connected()) {
      if (client.available()) {
        bodyBuffer[bytesRead++] = client.read();
      }
    }
    bodyBuffer[contentLength] = '\0';
    req.body = String(bodyBuffer);
    delete[] bodyBuffer;
  }
  
  Response res(client);
  
  // WebSocket support - check for WebSocket upgrade request
  if (_wsEnabled && req.method == GET && req.path == _wsPath && 
      req.headers.find("Upgrade") != req.headers.end() && 
      req.headers["Upgrade"] == "websocket") {
    // Respond with a note that we're handling this on port 81
    res.status(200).send("WebSocket service is available on port 81. Please connect to: ws://" + WiFi.localIP().toString() + ":81" + _wsPath);
    return;
  }
  
  // CORS handling.
  if (_corsEnabled) {
    res.setHeader("Access-Control-Allow-Origin", _corsOrigin);
    res.setHeader("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, OPTIONS");
    res.setHeader("Access-Control-Allow-Headers", "Content-Type");
    if (req.method == OPTIONS) {
      res.status(204).end();
      return;
    }
  }
  
  if (!_middleware.empty()) {
    applyMiddleware(req, res, 0);
  } else {
    handleRequest(req, res);
  }
}

void ESPExpress::applyMiddleware(Request& req, Response& res, size_t index) {
  if (index < _middleware.size()) {
    _middleware[index](req, res, [this, &req, &res, index]() {
      applyMiddleware(req, res, index + 1);
    });
  } else {
    handleRequest(req, res);
  }
}

void ESPExpress::handleRequest(Request& req, Response& res) {
  // First, try serving a static file.
  if (serveStaticFile(req, res)) {
    return;
  }
  
  // Look for a matching route.
  for (auto const &route : _routes) {
    std::map<String, String> routeParams;
    if (route.matches(req.method, req.path, routeParams)) {
      req.params = routeParams;
      route.handler(req, res);
      return;
    }
  }
  res.status(404).send("<h1>404 Not Found</h1><p>The requested resource was not found on this server.</p>");
}

bool ESPExpress::serveStaticFile(Request& req, Response& res) {
  if (req.method != GET) return false;
  
  for (auto const &entry : _staticPaths) {
    String virtualPath = entry.first;
    String fsPath = entry.second;
    if (req.path.startsWith(virtualPath)) {
      String relativePath = req.path.substring(virtualPath.length());
      if (relativePath.startsWith("/")) {
        relativePath = relativePath.substring(1);
      }
      String fullPath = fsPath;
      if (!fsPath.endsWith("/") && !relativePath.startsWith("/")) {
        fullPath += "/";
      }
      fullPath += relativePath;
      res.sendFile(fullPath);
      return true;
    }
  }
  return false;
}

Response& ESPExpress::render(Response& res, const String& templatePath, std::map<String, String> vars) {
  if (SPIFFS.exists(templatePath)) {
    File file = SPIFFS.open(templatePath, "r");
    if (file) {
      String content = file.readString();
      file.close();
      // Replace variables in template of form {{var}}
      for (auto const &v : vars) {
        String placeholder = "{{" + v.first + "}}";
        content.replace(placeholder, v.second);
      }
      res.send(content);
    } else {
      res.status(500).send("Failed to open template file");
    }
  } else {
    res.status(404).send("Template not found");
  }
  return res;
}