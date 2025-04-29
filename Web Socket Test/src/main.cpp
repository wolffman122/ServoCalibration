#include <WiFi.h>
#include <WebServer.h>
#include <WebSocketsServer.h>
#include <ArduinoJson.h>

WebServer server(80);
WebSocketsServer webSocket = WebSocketsServer(81);

int interval = 10000;
unsigned long previousMillis = 0;

StaticJsonDocument<200> docTX;
StaticJsonDocument<200> docRX;

void webSocketEvent(byte num, WStype_t type, uint8_t *payload, size_t length)
{
  switch (type)
  {
  case WStype_DISCONNECTED:
    Serial.println("Client Disconnected");
    break;
  case WStype_CONNECTED:
    Serial.println("Client Connected");
    break;
  case WStype_TEXT:
    DeserializationError error = deserializeJson(docRX, payload);

    if (error)
    {
      Serial.println("deserialize failed");
      return;
    }
    else
    {
      const char *g_brand = docRX["brand"];
      const char *g_type = docRX["type"];
      const int g_year = docRX["year"];
      const char *g_color = docRX["color"];

      Serial.println("Received Guitar info!");
      Serial.println("Brand: " + String(g_brand));
      Serial.println("Type: " + String(g_type));
      Serial.println("Year: " + String(g_year));
      Serial.println("Color: " + String(g_color));
    }
    break;
  }
}

void setup()
{
  Serial.begin(115200);

  WiFi.begin("Wolffden", "wolffresidence1322");
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
  }
  Serial.println("Connected to WiFi");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  server.begin();
  webSocket.begin();
  webSocket.onEvent(webSocketEvent);
}

void loop()
{
  server.handleClient();
  webSocket.loop();

  unsigned long now = millis();
  if (now - previousMillis > interval)
  {
    String str = String(random(100));
    String jsonString = "";
    JsonArray dataArray = docTX.createNestedArray("data");

    JsonObject obj1 = dataArray.createNestedObject();
    obj1["minimum"] = 0;
    obj1["maximum"] = 600;

    JsonObject obj2 = dataArray.createNestedObject();
    obj2["minimum"] = 0;
    obj2["maximum"] = 600;

    serializeJson(docTX, jsonString);
    Serial.println(jsonString);
    webSocket.broadcastTXT(jsonString);
    previousMillis = now;
  }
}