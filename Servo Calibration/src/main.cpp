#include <Wire.h>
#include <WiFi.h>
#include <WebSocketsServer.h>

#include <ArduinoJson.h>
#include <Preferences.h>
#include "WaveGenerator.h"
#include "ServoDriver.h"

std::shared_ptr<IServoDriver> spServoDriver;

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

WaveGenerator *waveGenerator;

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
  startWave,
  stopWave,
  addWaveServo,
  removeWaveServo,
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
  else if (actionStr == "startWave")
    return commands::startWave;
  else if (actionStr == "stopWave")
    return commands::stopWave;
  else if (actionStr == "addServo")
    return commands::addWaveServo;
  else if (actionStr == "removeServo")
    return commands::removeWaveServo;
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
    spServoDriver->setPWM(i, 0, servoMinimums[i]);
    servoPositions[i] = map(servoMinimums[i], servoMinimums[i], servoMaximums[i], 0, 180);
    Serial.println("Minimum Servo " + String(i) + ": " + String(servoPositions[i]));
  }
}

void CenterAllServos()
{
  for (int i = 0; i < 12; i++)
  {
    int target = servoMinimums[i] + (servoMaximums[i] - servoMinimums[i]) / 2;
    spServoDriver->setPWM(i, 0, target);
    servoPositions[i] = map(target, servoMinimums[i], servoMaximums[i], 0, 180);
    Serial.println("Centered Servo " + String(i) + ": " + String(servoPositions[i]));
  }
}

void MaximumAllServos()
{
  for (int i = 0; i < 12; i++)
  {
    spServoDriver->setPWM(i, 0, servoMaximums[i]);
    servoPositions[i] = map(servoMaximums[i], servoMinimums[i], servoMaximums[i], 0, 180);
    Serial.println("Maximum Servo " + String(i) + ": " + String(servoPositions[i]));
  }
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
      case commands::startWave:
        // Make all servos wave
        waveGenerator->StartWave();
        break;
      case commands::stopWave:
        // Stop the wave animation
        waveGenerator->StopWave();
        break;
      case commands::addWaveServo:
        // Add a servo to the wave animation
        waveGenerator->AddWaveServo();
        break;
      case commands::removeWaveServo:
        // Remove a servo from the wave animation
        waveGenerator->RemoveWaveServo();
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
          spServoDriver->setPWM(servoNumber, 0, servoMinimums[servoNumber]);
        }
        break;
      case commands::maxPWMChange:
        Serial.println("Received maxPWMChange action.");
        {
          JsonArray dataArray = docRX["data"];
          const int servoNumber = (int)(dataArray[0]["servoNumber"]);
          servoMaximums[servoNumber] = (int)(dataArray[0]["maximum"]);
          spServoDriver->setPWM(servoNumber, 0, servoMaximums[servoNumber]);
        }
        break;
      case commands::setAllToPWM:
        Serial.println("Received setAllToPWM action.");
        {
          boolean minimum = docRX["minimum"].as<boolean>();
          for (int i = 0; i < 12; i++)
          {
            int target = minimum ? servoMinimums[i] : servoMaximums[i];
            spServoDriver->setPWM(i, 0, target);
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
          spServoDriver->setPWM(servoNumber, 0, pwmPosition);
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

  spServoDriver = std::make_shared<ServoDriver>();
  waveGenerator = new WaveGenerator(*spServoDriver, servoPositions, servoMinimums, servoMaximums);

  webSocket.begin();
  webSocket.onEvent(webSocketEvent);

  // LoadValues();

  Serial.println("Setup Done");
}

void loop()
{
  if (waveGenerator->isWaveStarted())
    waveGenerator->UpdateWave();

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
