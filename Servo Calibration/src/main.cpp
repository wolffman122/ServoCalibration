#include <Wire.h>
#include <WiFi.h>
#include <WebSocketsServer.h>
#include <Adafruit_PWMServoDriver.h>
#include <ArduinoJson.h>

Adafruit_PWMServoDriver pca9685 = Adafruit_PWMServoDriver(0x40);
WebSocketsServer webSocket = WebSocketsServer(81);

int interval = 10000;
unsigned long previousMillis = 0;

// Wifi Setup
const char *ssid = "Wolffden";
const char *password = "wolffresidence1322";
// WiFiServer server(80);

// Current Time
unsigned long currentTime = millis();
// Previous Time
unsigned long previousTime = 0;
// Define timeout time in milliseconds
const long timeoutTime = 2000;

int servoMinimums[12] = {75, 75, 75, 75, 75, 75, 75, 75, 75, 75, 75, 75};
int servoMaximums[12] = {600, 600, 600, 600, 600, 600, 600, 600, 600, 600, 600, 600};
int servoPositions[12] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
float servoConversions[12] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
int sleepAngle[12] = {90, 150, 0, 90, 30, 180, 90, 150, 0, 90, 30, 180};

StaticJsonDocument<200> docTX;
StaticJsonDocument<200> docRX;

void initWiFi()
{
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.print("Connectiong to WiFi ..");
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print('.');
    delay(1000);
  }

  Serial.println(WiFi.localIP());
}

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
      for (int i = 0; i < 12; i++)
      {
        Serial.println((int)(docRX[i]["minimum"]));
        servoMinimums[i] = (int)(docRX[i]["minimum"]);
        servoMaximums[i] = (int)(docRX[i]["maximum"]);
        servoPositions[i] = (int)(docRX[i]["position"]);
        // const int g_Position = docRX[0]["position"];
        // Serial.println("Position: " + String(g_Position));
      }
    }
    break;
  }
}

void setup()
{
  Serial.begin(115200);
  initWiFi();

  Serial.println("PCA9685 Servo Test");

  bool wireBeginRet = Wire.begin(20, 21);
  Serial.println("Wire Begin " + wireBeginRet);
  pca9685.begin();

  pca9685.setPWMFreq(50);

  webSocket.begin();
  webSocket.onEvent(webSocketEvent);
}

void loop()
{
  webSocket.loop();

  unsigned long now = millis();
  if (now - previousMillis > interval)
  {
    String str = String(random(100));
    String jsonString = "";
    JsonArray dataArray = docTX.createNestedArray("data");

    for (int i = 0; i < 12; i++)
    {
      JsonObject obj = dataArray.createNestedObject();
      obj["servoNumber"] = i;
      obj["minimum"] = servoMinimums[i];
      obj["maximum"] = servoMaximums[i];
      obj["position"] = servoPositions[i];
    }

    serializeJson(docTX, jsonString);
    Serial.println(jsonString);
    webSocket.broadcastTXT(jsonString);
    previousMillis = now;
  }
}
