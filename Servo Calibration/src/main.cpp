#include <Wire.h>
#include <WiFi.h>
#include <WebSocketsServer.h>
#include <Adafruit_PWMServoDriver.h>
#include <ArduinoJson.h>
#include <Preferences.h>

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

Preferences preferences;

int servoMinimums[12] = {75, 75, 75, 75, 75, 75, 75, 75, 75, 75, 75, 75};
int servoMaximums[12] = {500, 500, 500, 500, 500, 500, 500, 500, 500, 500, 500, 500};
int servoPositions[12] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
int servoTotalRange[12] = {180, 180, 180, 180, 180, 180, 180, 180, 180, 180, 180, 180};
float servoConversions[12] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
int sleepAngle[12] = {90, 150, 0, 90, 30, 180, 90, 150, 0, 90, 30, 180};
int adjustedMinPWM[12] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
int adjustedMaxPWM[12] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

StaticJsonDocument<200> docTX;
StaticJsonDocument<200> docRX;

enum class commands
{
  minimum,
  center,
  maximum,
  wave,
  updateServoData,
  minPWMChange,
  maxPWMChange,
  setAllToPWM,
  totalRangeChange,
  updatePositionData,
  unknown
};

commands getCommandFromString(const String &actionStr)
{
  if (actionStr == "minimum")
    return commands::minimum;
  else if (actionStr == "center")
    return commands::center;
  else if (actionStr == "maximum")
    return commands::maximum;
  else if (actionStr == "wave")
    return commands::wave;
  else if (actionStr == "updateServoData")
    return commands::updateServoData;
  else if (actionStr == "minPWMChange")
    return commands::minPWMChange;
  else if (actionStr == "maxPWMChange")
    return commands::maxPWMChange;
  else if (actionStr == "setAllToPWM")
    return commands::setAllToPWM;
  else if (actionStr == "totalRangeChange")
    return commands::totalRangeChange;
  else if (actionStr == "updatePositionData")
    return commands::updatePositionData;
  else
    return commands::unknown;
}

void MinimumAllServos()
{
  for (int i = 0; i < 12; i++)
  {
    pca9685.setPWM(i, 0, servoMinimums[i]);
    servoPositions[i] = map(servoMinimums[i], servoMinimums[i], servoMaximums[i], 0, 180);
    Serial.println("Minimum Servo " + String(i) + ": " + String(servoPositions[i]));
  }
}

void CenterAllServos()
{
  for (int i = 0; i < 12; i++)
  {
    int target = servoMinimums[i] + (servoMaximums[i] - servoMinimums[i]) / 2;
    pca9685.setPWM(i, 0, target);
    servoPositions[i] = map(target, servoMinimums[i], servoMaximums[i], 0, 180);
    Serial.println("Centered Servo " + String(i) + ": " + String(servoPositions[i]));
  }
}

void MaximumAllServos()
{
  for (int i = 0; i < 12; i++)
  {
    pca9685.setPWM(i, 0, servoMaximums[i]);
    servoPositions[i] = map(servoMaximums[i], servoMinimums[i], servoMaximums[i], 0, 180);
    Serial.println("Maximum Servo " + String(i) + ": " + String(servoPositions[i]));
  }
}

void StartWave()
{
}

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

void StoreValues()
{
  preferences.begin("servo-config", false);

  for (int i = 0; i < 12; i++)
  {
    preferences.putInt(("min" + String(i)).c_str(), servoMinimums[i]);
    preferences.putInt(("max" + String(i)).c_str(), servoMaximums[i]);
    preferences.putInt(("totalRange" + String(i)).c_str(), servoTotalRange[i]);
  }

  preferences.end();
}

void LoadValues()
{
  preferences.begin("servo-config", true);

  for (int i = 0; i < 12; i++)
  {
    servoMinimums[i] = preferences.getInt(("min" + String(i)).c_str(), 75);
    servoMaximums[i] = preferences.getInt(("max" + String(i)).c_str(), 550);
    servoTotalRange[i] = preferences.getInt(("totalRange" + String(i)).c_str(), 180);
  }

  preferences.end();
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
    Serial.println("Received Text: " + String((const char *)payload, length));
    DeserializationError error = deserializeJson(docRX, payload);

    if (error)
    {
      Serial.println("deserialize failed");
      return;
    }

    if (docRX.containsKey("action"))
    {
      String actionStr = docRX["action"].as<String>();
      commands action = getCommandFromString(actionStr);
      Serial.println("Action to be taken " + actionStr);
      switch (action)
      {
      case commands::minimum:
        // Set all servos to minimum
        MinimumAllServos();
        break;
      case commands::center:
        // Center all servos
        CenterAllServos();
        break;
      case commands::maximum:
        // Set all servos to maximum
        MaximumAllServos();
        break;
      case commands::wave:
        // Make all servos wave
        StartWave();
        break;
      case commands::updateServoData:
        Serial.println("Received updateServoData action.");
        {
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
        break;
      case commands::minPWMChange:
        Serial.println("Received minPWMChange action.");
        {
          JsonArray dataArray = docRX["data"];
          const int servoNumber = (int)(dataArray[0]["servoNumber"]);
          servoMinimums[servoNumber] = (int)(dataArray[0]["minimum"]);
          pca9685.setPWM(servoNumber, 0, servoMinimums[servoNumber]);
        }
        break;
      case commands::maxPWMChange:
        Serial.println("Received maxPWMChange action.");
        {
          JsonArray dataArray = docRX["data"];
          const int servoNumber = (int)(dataArray[0]["servoNumber"]);
          servoMaximums[servoNumber] = (int)(dataArray[0]["maximum"]);
          pca9685.setPWM(servoNumber, 0, servoMaximums[servoNumber]);
        }
        break;
      case commands::setAllToPWM:
        Serial.println("Received setAllToPWM action.");
        {
          boolean minimum = docRX["minimum"].as<boolean>();
          for (int i = 0; i < 12; i++)
          {
            int target = minimum ? servoMinimums[i] : servoMaximums[i];
            pca9685.setPWM(i, 0, target);
          }
        }
        break;
      case commands::totalRangeChange:
        Serial.println("Received totalRangeChange action.");
        {
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
        break;
      case commands::updatePositionData:
        Serial.println("Received updatePositionData action.");
        {
          JsonArray dataArray = docRX["data"];
          const int servoNumber = (int)(dataArray[0]["servoNumber"]);
          const int position = (int)(dataArray[0]["position"]);
          servoPositions[servoNumber] = position;
          Serial.println("Received position update for servo " + String(servoNumber) + ": " + String(position) + ": " + String(servoPositions[servoNumber]));
          long pwmPosition = map(position, 0, 180, servoMinimums[servoNumber], servoMaximums[servoNumber]);
          pca9685.setPWM(servoNumber, 0, pwmPosition);
        }
        break;
      default:
        Serial.print("Unknown action received: ");
        Serial.println(actionStr);
        break;
      }
      StoreValues();
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

  Serial.println("PCA9685 initialized");
  pca9685.setPWMFreq(50);
  Serial.println("PCA9685 frequency set to 50Hz");

  webSocket.begin();
  webSocket.onEvent(webSocketEvent);

  // LoadValues();

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
    docTX.clear(); // Always clear before reuse!.
    JsonArray dataArray = docTX.to<JsonArray>();

    for (int i = 0; i < 12; i++)
    {
      JsonObject obj = dataArray.add<JsonObject>();
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
