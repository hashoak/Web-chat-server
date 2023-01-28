// Import required libraries
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>
#include <FS.h>

// Replace with your network credentials
const char* ssid = "Chat Server";
const char* password = "idontknow";

const int ledPin = BUILTIN_LED;

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

DynamicJsonDocument clnt(1024);
DynamicJsonDocument srvr(1024);
DynamicJsonDocument clntList(1024);
String x,y;

void notifyClients(uint32_t id, String time="", String message="") {
  x = "id" + String(id);
  Serial.printf("%s  %s: %s\n", time, String(clntList[x]), message);
  srvr["name"] = clntList[x];
  srvr["time"] = time;
  srvr["msg"] = message;
  x = "";
  serializeJson(srvr, x);
  ws.textAll(x);
}

void handleWebSocketMessage(AsyncWebSocketClient *client, void *arg, uint8_t *data, size_t len) {
  AwsFrameInfo *info = (AwsFrameInfo*)arg;
  if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT) {
    data[len] = 0;
    deserializeJson(clnt, (char*)data);
    if(clnt["name"])
    {
      x = "id" + String(client->id());
      clntList[x]=String(clnt["name"]);
      notifyClients(client->id(),clnt["time"]);
    }
    else if (clnt["msg"] && clnt["time"])
    {
      notifyClients(client->id(), clnt["time"], clnt["msg"]);
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
      notifyClients(client->id());
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

IPAddress local_IP(1,2,3,4);
IPAddress gateway(192,168,4,9);
IPAddress subnet(255,255,255,0);

void setup() {
  // Serial port for debugging purposes
  Serial.begin(115200);
  SPIFFS.begin();

  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, 0);

  // Initialize Wi-Fi
  Serial.println();
  Serial.print("Configuring soft-AP ... ");
  Serial.println(WiFi.softAPConfig(local_IP,gateway,subnet) ? "":"");
  Serial.print("Setting soft-AP: ");
  Serial.println(WiFi.softAP(ssid, password,1,false,8) ? "Ready" : "Failed!");
  Serial.print("IP address of soft-AP: ");
  Serial.println(WiFi.softAPIP());

  initWebSocket();

  // Route for root / web page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(SPIFFS, "/index.html","text/html");
  });
  server.on("/style.css", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(SPIFFS, "/style.css", "text/css");
  });
  server.on("/script.js", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(SPIFFS, "/script.js", "text/javascript");
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
