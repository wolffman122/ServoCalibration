#include <Arduino.h>
#include "WaveGenerator.h"

WaveGenerator::WaveGenerator(IServoDriver &servoDriver, int *servoPositions, int *servoMinimums, int *servoMaximums)
    : Generator(servoDriver, servoPositions, servoMinimums, servoMaximums)
{
  m_NumberOfServos = 4;
}

void WaveGenerator::Update()
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
      int position = m_Position + (i * 10 * m_Direction);
      if (position < 0)
        position = 0;
      else if (position > 180)
        position = 180;

      int pwm = map(position, 0, 180, m_ServoMinimums[i], m_ServoMaximums[i]);
      m_ServoDriver.setPWM(i, 0, pwm);
      m_ServoPositions[i] = m_Position;
    }

    m_LastUpdate = now;
  }
}