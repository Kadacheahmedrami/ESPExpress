#ifndef ESP_EXPRESS_H
#define ESP_EXPRESS_H

#include <WiFi.h>
#include <map>
#include <vector>
#include <functional>
#include <SPIFFS.h>

// Add WebSocket library
#include <WebSocketsServer.h>

// HTTP Methods
enum HttpMethod {
  GET,
  POST,
  PUT,
  DELETE,
  OPTIONS
};

// Convert HTTP method string to enum
HttpMethod stringToMethod(const String& method);
String methodToString(HttpMethod method);

// Request class to hold information about the HTTP request
class Request {
public:
  HttpMethod method;
  String path;
  String version;
  std::map<String, String> headers;
  std::map<String, String> query;
  String body;
  std::map<String, String> params; // Route parameters

  Request(const String& requestLine);
  void addHeader(const String& line);
  void parseQuery();
  String getParam(const String& name) const;
  String getHeader(const String& name) const;
  String getQuery(const String& name) const;
};

// Response class to construct and send HTTP responses
class Response {
public:
  WiFiClient& client;
  bool headersSent;
  int statusCode;
  std::map<String, String> headers;

  Response(WiFiClient& client);
  Response& status(int code);
  Response& setHeader(const String& name, const String& value);
  Response& send(const String& body);
  Response& sendJson(const String& json);
  Response& sendFile(const String& path);
  void end();

private:
  void sendHeaders();
};

// WebSocket callback function types
typedef std::function<void(uint8_t num, WStype_t type, uint8_t* payload, size_t length)> WebSocketEventCallback;
typedef std::function<void(uint8_t num, uint8_t* payload, size_t length)> WebSocketMessageCallback;
typedef std::function<void(uint8_t num)> WebSocketClientCallback;

// Middleware function type
typedef std::function<void(Request&, Response&, std::function<void()>)> MiddlewareFunction;

// Route handler function type
typedef std::function<void(Request&, Response&)> HandlerFunction;

// Route structure with path pattern matching
struct Route {
  HttpMethod method;
  String pattern;
  HandlerFunction handler;
  std::vector<String> paramNames; // Names of parameters in the route pattern
  
  bool matches(HttpMethod m, const String& path, std::map<String, String>& params) const;
};

class ESPExpress {
public:
  ESPExpress(uint16_t port = 80);
  ~ESPExpress();

  // HTTP method handlers
  ESPExpress& get(const String& path, HandlerFunction handler);
  ESPExpress& post(const String& path, HandlerFunction handler);
  ESPExpress& put(const String& path, HandlerFunction handler);
  ESPExpress& del(const String& path, HandlerFunction handler); // 'delete' is a keyword
  ESPExpress& options(const String& path, HandlerFunction handler);
  
  // Route for all methods
  ESPExpress& all(const String& path, HandlerFunction handler);
  
  // Middleware
  ESPExpress& use(MiddlewareFunction middleware);
  
  // Static file serving
  ESPExpress& serveStatic(const String& virtualPath, const String& fsPath);
  
  // Start the server
  void listen(const char* message = nullptr);
  
  // Template for serving HTML with variables
  Response& render(Response& res, const String& templatePath, std::map<String, String> vars);
  
  // CORS support
  ESPExpress& enableCORS(const String& origin = "*");
  
  // WebSocket support - new methods
  ESPExpress& ws(const String& path, WebSocketEventCallback callback);
  ESPExpress& onWsConnect(WebSocketClientCallback callback);
  ESPExpress& onWsDisconnect(WebSocketClientCallback callback);
  ESPExpress& onWsMessage(WebSocketMessageCallback callback);
  
  // WebSocket broadcast and send methods
  void wsBroadcast(uint8_t* payload, size_t length);
  void wsBroadcastTXT(const String& text);
  void wsSend(uint8_t num, uint8_t* payload, size_t length);
  void wsSendTXT(uint8_t num, const String& text);
  
  // WebSocket loop method - must be called in Arduino loop()
  void wsLoop();
  
private:
  WiFiServer _server;
  std::vector<Route> _routes;
  std::vector<MiddlewareFunction> _middleware;
  std::map<String, String> _staticPaths;
  bool _corsEnabled;
  String _corsOrigin;
  
  // WebSocket server - new private members
  WebSocketsServer* _webSocketServer;
  bool _wsEnabled;
  String _wsPath;
  WebSocketEventCallback _wsEventCallback;
  WebSocketClientCallback _wsConnectCallback;
  WebSocketClientCallback _wsDisconnectCallback;
  WebSocketMessageCallback _wsMessageCallback;
  
  void processClient(WiFiClient& client);
  void handleRequest(Request& req, Response& res);
  bool serveStaticFile(Request& req, Response& res);
  void applyMiddleware(Request& req, Response& res, size_t index);
  
  // WebSocket internal event handler
  void handleWebSocketEvent(uint8_t num, WStype_t type, uint8_t* payload, size_t length);
};

// Utility functions
String urlDecode(const String& text);
String getContentType(const String& path);
bool pathMatch(const String& pattern, const String& path, std::map<String, String>& params);

#endif