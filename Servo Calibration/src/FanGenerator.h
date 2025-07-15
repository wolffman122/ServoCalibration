#include "IGenerator.h"
#include "IServoDriver.h"

class FanGenerator: public IGenerator
{
private:
  bool waveStarted = false;
  int wavePosition = 0;
  int waveDirection = 1; // 1 = increasing, -1 = decreasing
  unsigned long lastWaveUpdate = 0;
  const int waveStep = 2;                // degrees per update
  const unsigned long waveInterval = 20; // ms between updates
  int numberOfWaveServos = 1;            // Number of servos in the wave
  IServoDriver &m_ServoDriver;
  int *servoMinimums = nullptr;
  int *servoMaximums = nullptr;
  int *servoPositions = nullptr;

public:
  FanGenerator(IServoDriver &servoDriver, int *servoPositions, int *servoMinimums, int *servoMaximums);

  virtual void Update() override;
  virtual void Start() override;
  virtual void Stop() override;

  virtual bool isStarted() const override;
  virtual void AddServo() override;
  virtual void RemoveServo() override;
};