#include <Arduino.h>
#include "FanGenerator.h"

FanGenerator::FanGenerator(IServoDriver &servoDriver, int *servoPositions, int *servoMinimums, int *servoMaximums)
    : m_ServoDriver(servoDriver), servoPositions(servoPositions), servoMinimums(servoMinimums), servoMaximums(servoMaximums), waveStarted(false), wavePosition(0), waveDirection(1), lastWaveUpdate(0)
{
}

void FanGenerator::Start()
{
  waveStarted = true;
  wavePosition = 0;
  waveDirection = 1; // Start with increasing direction
  lastWaveUpdate = millis();
}

void FanGenerator::Stop()
{
  waveStarted = false;
}

bool FanGenerator::isStarted() const
{
  return waveStarted;
}

void FanGenerator::Update()
{
  unsigned long now = millis();
  if (now - lastWaveUpdate > waveInterval)
  {
    // Move position
    wavePosition += waveDirection * waveStep;
    if (wavePosition >= 180)
    {
      wavePosition = 180;
      waveDirection = -1; // Change direction to decreasing
    }
    else if (wavePosition <= 0)
    {
      wavePosition = 0;
      waveDirection = 1; // Change direction to increasing
    }

    for (int i = 0; i < numberOfWaveServos; i++)
    {
      int pwm = map(wavePosition, 0, 180, servoMinimums[i], servoMaximums[i]);
      m_ServoDriver.setPWM(i, 0, pwm);
      servoPositions[i] = wavePosition;
    }

    lastWaveUpdate = now;
  }
}

void FanGenerator::AddServo()
{
  (numberOfWaveServos < 12) ? numberOfWaveServos++ : Serial.println("Maximum number of wave servos reached.");
}

void FanGenerator::RemoveServo()
{
  (numberOfWaveServos > 1) ? numberOfWaveServos-- : Serial.println("Minimum number of wave servos is 1.");
}
