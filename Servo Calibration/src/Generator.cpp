#include <Arduino.h>
#include "Generator.h"

Generator::Generator(IServoDriver &servoDriver, WebSocketsServer *pWebSocket, int *servoPositions, int *servoMinimums, int *servoMaximums)
    : m_ServoDriver(servoDriver), m_pWebSocket(pWebSocket), m_ServoPositions(servoPositions), m_ServoMinimums(servoMinimums), m_ServoMaximums(servoMaximums)
{
}

void Generator::Start()
{
  m_Started = true;
  m_Position = 0;
  m_Direction = 1;
  m_LastUpdate = millis();

  m_pWebSocket->broadcastTXT("Generator Started");
}

void Generator::Stop()
{
  m_Started = false;
  m_pWebSocket->broadcastTXT("Generator Stopped");
}

bool Generator::isStarted() const
{
  return m_Started;
}

void Generator::AddServo()
{
  (m_NumberOfServos < 12) ? m_NumberOfServos++ : Serial.println("Maximum number of servos reached.");
  m_pWebSocket->broadcastTXT("Added Servo. Total: " + String(m_NumberOfServos));
}

void Generator::RemoveServo()
{
  (m_NumberOfServos > 1) ? m_NumberOfServos-- : Serial.println("Minimum number of servos is 1.");
  m_pWebSocket->broadcastTXT("Removed Servo. Total: " + String(m_NumberOfServos));
}
