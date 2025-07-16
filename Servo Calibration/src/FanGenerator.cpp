#include <Arduino.h>
#include "FanGenerator.h"

FanGenerator::FanGenerator(IServoDriver &servoDriver, WebSocketsServer *pWebSocket, int *servoMinimums, int *servoMaximums)
    : Generator(servoDriver, pWebSocket, servoMinimums, servoMaximums)
{
}

void FanGenerator::Update()
{
  unsigned long now = millis();
  if (now - m_LastUpdate > m_Interval)
  {
    // Move position
    m_Position += m_Direction * m_Step;
    if (m_Position >= 180)
    {
      m_Position = 180;
      m_Direction = -1; // Change direction to decreasing
    }
    else if (m_Position <= 0)
    {
      m_Position = 0;
      m_Direction = 1; // Change direction to increasing
    }

    for (int i = 0; i < m_NumberOfServos; i++)
    {
      int pwm = map(m_Position, 0, 180, m_ServoMinimums[i], m_ServoMaximums[i]);
      m_ServoDriver.setPWM(i, 0, pwm);
    }

    m_LastUpdate = now;
  }
}