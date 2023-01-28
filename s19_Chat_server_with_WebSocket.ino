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
String x,msg, id;
uint32_t count;

void notifyClients(String id, String time = "", String message = "",String img="") {
  srvr["name"] = clntList[id]["name"];
  srvr["time"] = time;
  srvr["msg"] = message;
  srvr["img"] = img;
  x = "";
  serializeJson(srvr, x);
  ws.textAll(x);
}

void handleWebSocketMessage(AsyncWebSocketClient *client, void *arg, uint8_t *data, size_t len) {
  AwsFrameInfo *info = (AwsFrameInfo*)arg;
  if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT) {
    data[len] = 0;
    deserializeJson(clnt, (char*)data);
    serializeJson(clnt,Serial);
    id = String(client->id());
    if (clnt["name"]) // Just after client connects to the server
    {
      clntList[id]["stat"] = true;
      clntList[id]["name"] = String(clnt["name"]);
      Serial.printf("Client ID: %s -> Name: %s\n", id, String(clntList[id]["name"]));
      notifyClients(id, clnt["time"]);
      msg = "";
      for (int i = 1; i < count; i++)
      {
        x=String(i);
        if (boolean(clntList[x]["stat"])) msg += String(clntList[x]["name"]);
        if (boolean(clntList[x]["stat"]) && i < count - 1) msg += ", ";
      }
      if (msg == "") msg = "No one";
      if(msg[msg.length()-1]==' ') msg[msg.length()-2]='\0';
      srvr["name"] = "";
      srvr["time"] = "";
      srvr["msg"] = msg;
      x = "";
      serializeJson(srvr, x);
      ws.text(client->id(), x);
    }
    else if (clnt["msg"] && clnt["time"])
    {
      Serial.printf("%s  %s: %s\n", String(clnt["time"]), String(clntList[id]["name"]), String(clnt["msg"]));
      notifyClients(id, clnt["time"], clnt["msg"],clnt["img"]);
    }
  }
}

void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len) {
  switch (type) {
    case WS_EVT_CONNECT:
      Serial.printf("Client ID:%u connected\n", client->id());
      count++;
      break;
    case WS_EVT_DISCONNECT:
      id = String(client->id());
      clntList[id]["stat"] = false;
      Serial.printf("Client ID:%s, Name: %s disconnected\n", id, String(clntList[id]["name"]));
      notifyClients(id);
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

IPAddress local_IP(1, 2, 3, 4);
IPAddress gateway(192, 168, 4, 9);
IPAddress subnet(255, 255, 255, 0);

void setup() {
  // Serial port for debugging purposes
  Serial.begin(115200);
  SPIFFS.begin();

  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, 0);

  // Initialize Wi-Fi
  Serial.println();
  Serial.print("Configuring soft-AP ... ");
  Serial.println(WiFi.softAPConfig(local_IP, gateway, subnet) ? "" : "");
  Serial.print("Setting soft-AP: ");
  Serial.println(WiFi.softAP(ssid, password, 1, false, 8) ? "Ready" : "Failed!");
  Serial.print("IP address of soft-AP: ");
  Serial.println(WiFi.softAPIP());

  initWebSocket();

  // Route for root / web page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(SPIFFS, "/index.html", "text/html");
  });
  server.on("/style.css", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(SPIFFS, "/style.css", "text/css");
  });
  server.on("/script.js", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(SPIFFS, "/script.js", "text/javascript");
  });

  // Start server
  server.begin();
  count=0;
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
    ser = "";
    serializeJson(srvr, ser);
    ws.textAll(ser);
  }
  ws.cleanupClients();
}
