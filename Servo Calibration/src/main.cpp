#include <Wire.h>
#include <WiFi.h>
#include <WebSocketsServer.h>

#include <ArduinoJson.h>
#include <Preferences.h>
#include "FanGenerator.h"
#include "WaveGenerator.h"
#include "ServoDriver.h"

std::shared_ptr<IServoDriver> spServoDriver;

WebSocketsServer *pwebSocketServer = new WebSocketsServer(81);

int interval = 10000;
unsigned long previousMillis = 0;

// Wifi Setup
const char *ssid = "Wolffden";
const char *password = "wolffresidence1322";
// WiFiServer server(80);

std::shared_ptr<IGenerator> spGenerator;

int servoMinimums[12] = {75, 75, 75, 75, 75, 75, 75, 75, 75, 75, 75, 75};
int servoMaximums[12] = {500, 500, 500, 500, 500, 500, 500, 500, 500, 500, 500, 500};

JsonDocument docTX;
JsonDocument docRX;

enum class commands
{
  minimum,
  center,
  maximum,
  startWave,
  stopWave,
  startFan,
  stopFan,
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
  else if (actionStr == "startFan")
    return commands::startFan;
  else if (actionStr == "stopFan")
    return commands::stopFan;
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
  }

  pwebSocketServer->broadcastTXT("Minimum Servos");
}

void CenterAllServos()
{
  for (int i = 0; i < 12; i++)
  {
    int target = servoMinimums[i] + (servoMaximums[i] - servoMinimums[i]) / 2;
    spServoDriver->setPWM(i, 0, target);
  }

  pwebSocketServer->broadcastTXT("Centered Servos");
}

void MaximumAllServos()
{
  for (int i = 0; i < 12; i++)
  {
    spServoDriver->setPWM(i, 0, servoMaximums[i]);
  }

  pwebSocketServer->broadcastTXT("Maximum Servos");
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
      case commands::startFan:
        // Switch to FanGenerator and start
        spGenerator = std::make_shared<FanGenerator>(*spServoDriver, pwebSocketServer, servoMinimums, servoMaximums);
        spGenerator->Start();
        break;
      case commands::stopFan:
        // Stop the current generator
        if (spGenerator)
          spGenerator->Stop();
        break;
      case commands::startWave:
        // Switch to WaveGenerator and start
        spGenerator = std::make_shared<WaveGenerator>(*spServoDriver, pwebSocketServer, servoMinimums, servoMaximums);
        spGenerator->Start();
        break;
      case commands::stopWave:
        // Stop the current generator
        if (spGenerator)
          spGenerator->Stop();
        break;
      case commands::addWaveServo:
        // Add a servo to the wave animation
        spGenerator->AddServo();
        break;
      case commands::removeWaveServo:
        // Remove a servo from the wave animation
        spGenerator->RemoveServo();
        break;
      default:
        Serial.println("Unknown action: " + actionStr);
        break;
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

  spServoDriver = std::make_shared<ServoDriver>();
  spGenerator = std::make_shared<FanGenerator>(*spServoDriver, pwebSocketServer, servoMinimums, servoMaximums);

  pwebSocketServer->begin();
  pwebSocketServer->onEvent(webSocketEvent);

  Serial.println("Setup Done");
}

void loop()
{
  if (spGenerator->isStarted())
    spGenerator->Update();

  pwebSocketServer->loop();

  unsigned long now = millis();
  if (now - previousMillis > interval)
  {
    
    previousMillis = now;
  }
}
