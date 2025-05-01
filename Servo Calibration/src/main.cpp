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
int servoTotalRange[12] = {180, 180, 180, 180, 180, 180, 180, 180, 180, 180, 180, 180};
float servoConversions[12] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
int sleepAngle[12] = {90, 150, 0, 90, 30, 180, 90, 150, 0, 90, 30, 180};
int adjustedMinPWM[12] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
int adjustedMaxPWM[12] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

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

    if (docRX.containsKey("action"))
    {
      String action = docRX["action"].as<String>();

      Serial.println("Action to be taken " + action);

      if (action == "updateServoData")
      {
        Serial.println("Received updateServoData action.");

        JsonArray dataArray = docRX["data"];
        for (int i = 0; i < dataArray.size(); i++)
        {
          servoMinimums[i] = (int)(dataArray[i]["minimum"]);
          servoMaximums[i] = (int)(dataArray[i]["maximum"]);
          servoPositions[i] = (int)(dataArray[i]["position"]);
          servoConversions[i] = (float)(dataArray[i]["conversionFactor"]);
          adjustedMinPWM[i] = (int)(dataArray[i]["adjustedMinPWM"]);
          adjustedMaxPWM[i] = (int)(dataArray[i]["adjustedMaxPWM"]);
        }
      }
      else if (action == "minPWMChange")
      {
        Serial.println("Received minPWMChange action.");
        JsonArray dataArray = docRX["data"];
        const int servoNumber = (int)(dataArray[0]["servoNumber"]);

        servoMinimums[servoNumber] = (int)(dataArray[0]["minimum"]);

        pca9685.setPWM(servoNumber, 0, servoMinimums[servoNumber]);
        // Need to add code to actually move the servo.
      }
      else if (action == "maxPWMChange")
      {
        Serial.println("Received maxPWMChange action.");
        JsonArray dataArray = docRX["data"];
        const int servoNumber = (int)(dataArray[0]["servoNumber"]);

        servoMaximums[servoNumber] = (int)(dataArray[0]["maximum"]);

        pca9685.setPWM(servoNumber, 0, servoMaximums[servoNumber]);
        // Need to add code to actually move the servo.
      }
      else if (action == "setAllToPWM")
      {
        Serial.println("Received setAllToPWM action.");

        boolean minimum = docRX["minimum"].as<boolean>();

        for (int i = 0; i < 12; i++)
        {
          int target = minimum ? servoMinimums[i] : servoMaximums[i];
          pca9685.setPWM(i, 0, target);
        }
      }
      else if (action == "totalRangeChange")
      {
        Serial.println("Received totalRangeChange action.");
        JsonArray dataArray = docRX["data"];
        const int servoNumber = (int)(dataArray[0]["servoNumber"]);

        servoTotalRange[servoNumber] = (int)(dataArray[0]["totalRange"]);

        servoConversions[servoNumber] = (float)(servoMaximums[servoNumber] - servoMinimums[servoNumber]) / servoTotalRange[servoNumber];

        const float extraRange = (servoTotalRange[servoNumber] - 180) / 2.0;

        adjustedMinPWM[servoNumber] = ceil((servoConversions[servoNumber] * extraRange) + servoMinimums[servoNumber]);
        adjustedMaxPWM[servoNumber] = ceil((servoConversions[servoNumber] * extraRange) + servoMaximums[servoNumber]);

        Serial.println(servoMaximums[servoNumber]);
        Serial.println(servoMinimums[servoNumber]);
        Serial.println(servoTotalRange[servoNumber]);
        Serial.println(servoConversions[servoNumber]);
        Serial.println(adjustedMinPWM[servoNumber]);
        Serial.println(adjustedMaxPWM[servoNumber]);
      }
      else if (action == "updatePositionData")
      {
        Serial.println("Received updatePositionData action.");

        JsonArray dataArray = docRX["data"];
        const int servoNumber = (int)(dataArray[0]["servoNumber"]);
        const int position = (int)(dataArray[0]["position"]);

        servoPositions[servoNumber] = position;

        long pwmPosition = map(position, 0, 180, servoMinimums[servoNumber], servoMaximums[servoNumber]);
        pca9685.setPWM(servoNumber, 0, pwmPosition);
      }
      else
      {
        Serial.print("Unknown action received: ");
        Serial.println(action);
      }
    }
    else
    {
      Serial.println("No action field found in JSON.");
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

  Serial.println("Setup Done");
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
      obj["totalRange"] = servoTotalRange[i];
      obj["conversionFactor"] = servoConversions[i];
      obj["adjustedMinPWM"] = adjustedMinPWM[i];
      obj["adjustedMaxPWM"] = adjustedMaxPWM[i];
    }

    serializeJson(docTX, jsonString);
    // Serial.println(jsonString);
    webSocket.broadcastTXT(jsonString);
    previousMillis = now;
  }
}
