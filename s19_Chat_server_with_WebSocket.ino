// Import required libraries
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>
#include <FS.h>

// Replace with your network credentials
const char* ssid = "Good Server";
const char* password = "idontknow";

const int ledPin = BUILTIN_LED;

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

DynamicJsonDocument clnt(1024);
DynamicJsonDocument srvr(1024);
DynamicJsonDocument clntList(1024);

void notifyClients(uint32_t id, String message, String time) {
  String x = "id" + String(id);
  Serial.printf("%s: %s\n", String(clntList[x]), message);
  srvr["name"] = clntList[x];
  srvr["msg"] = message;
  srvr["time"] = time;
  x = "";
  serializeJson(srvr, x);
  ws.textAll(x);
}

void handleWebSocketMessage(AsyncWebSocketClient *client, void *arg, uint8_t *data, size_t len) {
  AwsFrameInfo *info = (AwsFrameInfo*)arg;
  if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT) {
    data[len] = 0;
    deserializeJson(clnt, (char*)data);
    if (clnt["name"])
    {
      String x = "id" + String(client->id());
      String y = clnt["name"];
      clntList[x] = y;
      Serial.printf("%s -> %s\n", x, y);
    }
    else if (clnt["msg"] && clnt["time"])
    {
      notifyClients(client->id(), clnt["msg"], clnt["time"]);
    }
  }
}

void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len) {
  switch (type) {
    case WS_EVT_CONNECT:
      Serial.printf("WebSocket client #%u connected from %s\n", client->id(), client->remoteIP().toString().c_str());
      break;
    case WS_EVT_DISCONNECT:
      Serial.printf("WebSocket client #%u disconnected\n", client->id());
      break;
    case WS_EVT_DATA:
      handleWebSocketMessage(client, arg, data, len);
      break;
    case WS_EVT_PONG:
    case WS_EVT_ERROR:
      break;
  }
}

void initWebSocket() {
  ws.onEvent(onEvent);
  server.addHandler(&ws);
}

String processor(const String& var) {
  return String();
}

void setup() {
  // Serial port for debugging purposes
  Serial.begin(115200);
  SPIFFS.begin();

  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, 0);

  // Initialize Wi-Fi
  Serial.println();
  Serial.println("Setting soft-AP ... ");
  Serial.println(WiFi.softAP(ssid, password) ? "Ready" : "Failed!");
  Serial.print("AP IP address: ");
  Serial.println(WiFi.softAPIP());

  initWebSocket();

  // Route for root / web page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(SPIFFS, "/index.html", String(), false, processor);
  });
  server.on("/style.css", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(SPIFFS, "/style.css", "text/css");
  });

  // Start server
  server.begin();
  digitalWrite(ledPin, 1);
}

String ser;

void loop() {
  if (Serial.available())
  {
    ser = Serial.readStringUntil('\n');
    Serial.print("You: ");
    Serial.println(ser);
    srvr["name"] = "Server";
    srvr["msg"] = ser;
    srvr["time"] = "NA";
    ser="";
    serializeJson(srvr, ser);
    ws.textAll(ser);
  }
  ws.cleanupClients();
}
