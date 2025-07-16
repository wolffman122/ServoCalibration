#include <Arduino.h>
#include "Generator.h"

Generator::Generator(IServoDriver& servoDriver, int* servoPositions, int* servoMinimums, int* servoMaximums)
  : m_ServoDriver(servoDriver)
  , m_ServoPositions(servoPositions)
  , m_ServoMinimums(servoMinimums)
  , m_ServoMaximums(servoMaximums)
{
}

void Generator::Start()
{
  m_Started = true;
  m_Position = 0;
  m_Direction = 1;
  m_LastUpdate = millis();
}

void Generator::Stop()
{
  m_Started = false;
}

bool Generator::isStarted() const
{
  return m_Started;
}

void Generator::AddServo()
{
  (m_NumberOfServos < 12) ? m_NumberOfServos++ : Serial.println("Maximum number of servos reached.");
}

void Generator::RemoveServo()
{
  (m_NumberOfServos > 1) ? m_NumberOfServos-- : Serial.println("Minimum number of servos is 1.");
}
