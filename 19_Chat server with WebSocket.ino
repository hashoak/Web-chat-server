// Import required libraries
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>

// Replace with your network credentials
const char* ssid = "Good Server";
const char* password = "idontknow";

const int ledPin = BUILTIN_LED;

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
  <head>
    <title>OnlineChatServer</title>
  </head>
  <style>
    body {
      overflow: hidden;
      height: 100vw;
      display: flex;
      flex-direction: column;
      background-color: rgb(54, 54, 54);
    }
    .top {
      height: 50px;
      display: flex;
      background: rgb(255, 108, 108);
      color: aliceblue;
      justify-content: center;
    }
    h1 {
      margin: 0;
      align-self: center;
    }
    #conv {
      display: flex;
      padding: 0 20px 0 20px;
      margin: 20px 0 10px 0;
      overflow-y: scroll;
      height: 450px;
      color: aliceblue;
      align-self: left;
      flex-direction: column;
      font-family: Consolas, "Courier New", monospace;
    }
    ::-webkit-scrollbar {
      width: 6px;
    }
    ::-webkit-scrollbar-track {
      background: #666;
      border-radius: 30px;
    }
    ::-webkit-scrollbar-thumb {
      background: white;
      border-radius: 30px;
    }
    ::-webkit-scrollbar-thumb:hover {
      background: linear-gradient(skyblue, yellowgreen);
    }
    .text {
      display: block;
      flex-direction: column;
      width: auto;
      max-width: 80%;
      height: auto;
      margin: 3px;
    }
    #left {
      align-self: flex-start;
      display: flex;
      text-align: right;
      background-color: rgb(69, 69, 69);
    }
    #right {
      align-self: flex-end;
      text-align: right;
      padding: 10px;
      background-color: rgb(41, 105, 255);
    }
    .message {
      display: flex;
      text-align: center;
      height: 50px;
      padding: 2.5px;
      display: flex;
      justify-content: center;
    }
    #nick {
      width: 100px;
      height: 45px;
      margin: 0 10px 0 0;
    }
    #in {
      width: 390px;
      height: 45px;
    }
    #send {
      font-size: medium;
      font-weight: bold;
      color: whitesmoke;
      background-color: lightseagreen;
      border-color: red;
      width: 100px;
      height: 50px;
      border-radius: 18px;
    }
    #rcv {
      font-size: xx-small;
      color: red;
    }
    .name {
      font-size: large;
      align-self: flex-start;
      color: rgb(65, 235, 107);
      padding: 2px;
      margin: 2px 4px;
    }
    .txt {
      margin: 0px 9px 5px;
      font-size: medium;
      align-self: flex-start;
    }
    .time {
      color: #666;
      font-size: small;
    }
  </style>

  <body>
    <div class="top">
      <h1>HASH's Chat Room</h1>
    </div>
    <div id="conv">
      <div class="text" id="left">
        <div class="name">Server</div>
        <div class="txt">Hola!</div>
        <div class="time" id="time1"></div>
        <script>
          document.getElementById("time1").innerHTML =
            new Date().getHours() +
            ":" +
            (new Date().getMinutes() < 10 ? "0" : "") +
            new Date().getMinutes();
        </script>
      </div>
      <div class="text" id="left">
        <div class="name">Server</div>
        <div class="txt">Welcome to the private chat cloud.</div>
        <div class="time" id="time2">
          <script>
            document.getElementById("time2").innerHTML =
              new Date().getHours() +
              ":" +
              (new Date().getMinutes() < 10 ? "0" : "") +
              new Date().getMinutes();
          </script>
        </div>
      </div>
    </div>
    <div class="message">
      <!-- <input id="nick" type="textarea" placeholder="Your name..." /> -->
      <input id="in" type="textarea" placeholder="Write a message..." />
      <button id="send" onclick="sndMsg()">Send</button>
    </div>
  </body>

  <script>
    let gateway = `ws://${window.location.hostname}/ws`;
    let websocket, id, rec, ch;

    let initButton = () => {
      document.getElementById("send").addEventListener("keypress", (event) => {
        console.log(event);
        if (event.key == 13) {
          document.getElementById("send").click();
        }
      });
    };

    let onLoad = (event) => {
      initWebSocket();
      initButton();
    };

    window.addEventListener("load", onLoad);

    let initWebSocket = () => {
      console.log("Trying to open a WebSocket connection...");
      websocket = new WebSocket(gateway);
      websocket.onopen = onOpen;
      websocket.onclose = onClose;
      websocket.onmessage = onMessage; // <-- add this line
    };

    let onOpen = (event) => {
      console.log("Connection opened");
    };

    let onClose = (event) => {
      console.log("Connection closed");
      setTimeout(initWebSocket, 2000);
    };

    let onMessage = (event) => {
      rcv = JSON.parse(event.data);
      let flags = Boolean(rcv.name && rcv.message && rcv.time);

      if (rcv.stat == "init" && rcv.id) id = rcv.id;
      else if (rcv.stat == "msg" && rcv.id != id && flags) {
        document.getElementById("conv").innerHTML +=
          "<div class='text' id='left'><div class='name'>" +
          rcv.name +
          "</div><div class='txt'>" +
          rcv.message +
          "</div><div name='time'>" +
          rcv.time +
          "</div></div>";
      }
    };

    let sndReq = (obj) => {
      websocket.send(JSON.stringify(obj));
    };

    let sndFirstReq = () => {
      let name = prompt("Name Enter chey Bhayia:");
      const info = {
        name,
      };
      sndReq(info);
    };

    sndFirstReq();

    function sndMsg() {
      let time = new Date().getHours() + ":" + new Date().getMinutes();
      const info = {
        message: document.getElementById("in").value,
        time,
      };
      document.getElementById("in").value = "";
      document.getElementById("conv").innerHTML +=
        "<div class='text' id='right'>" +
        info.message +
        "<div class='time'>" +
        info.time +
        "</div></div>";
      sndReq(info);
    }
  </script>
</html>
)rawliteral";

DynamicJsonDocument clnt(1024);
DynamicJsonDocument srvr(1024);
DynamicJsonDocument clntList(1024);

void notifyClients(uint32_t id,String message,String time) {
  char *str;
  itoa(int(id),str,10);
  String x="id"+String(str);
  srvr["name"]=clntList[str];
  srvr["message"]=message;
  srvr["time"]=time;
  serializeJson(srvr,x);
  ws.textAll(x);
}

void handleWebSocketMessage(AsyncWebSocketClient *client,void *arg, uint8_t *data, size_t len) {
  AwsFrameInfo *info = (AwsFrameInfo*)arg;
  if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT) {
    data[len] = 0;
    deserializeJson(clnt,(char*)data);
    if(clnt["name"])
    {
      char *str;
      itoa(int(client->id()),str,10);
      String x="id"+String(str);
      clntList[x]=clnt["name"];
    }
    else if(clnt["msg"] && clnt["time"])
    {
      notifyClients(client->id(),clnt["name"],clnt["time"]);
    }
  }
}

void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type,void *arg, uint8_t *data, size_t len) {
    switch (type) {
      case WS_EVT_CONNECT:
        Serial.printf("WebSocket client #%u connected from %s\n", client->id(), client->remoteIP().toString().c_str());
        break;
      case WS_EVT_DISCONNECT:
        Serial.printf("WebSocket client #%u disconnected\n", client->id());
        break;
      case WS_EVT_DATA:
        handleWebSocketMessage(client,arg,data,len);
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

String processor(const String& var){
  Serial.println(var);
  return String();
}

void setup(){
  // Serial port for debugging purposes
  Serial.begin(115200);

  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin,0);
  
  // Initialize Wi-Fi
  Serial.println();
  Serial.println("Setting soft-AP ... ");
  Serial.println(WiFi.softAP(ssid,password) ? "Ready" : "Failed!");
  Serial.print("AP IP address: ");
  Serial.println(WiFi.softAPIP());

  initWebSocket();

  // Route for root / web page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", index_html, processor);
  });

  // Start server
  server.begin();
  digitalWrite(ledPin,1);
}

void loop() {
  ws.cleanupClients();
}