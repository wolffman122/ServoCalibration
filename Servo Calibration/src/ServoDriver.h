#include <Adafruit_PWMServoDriver.h>

#include "IServoDriver.h"

class ServoDriver : public IServoDriver
{
private:
  Adafruit_PWMServoDriver m_Pca9685;

public:
  ServoDriver();
  virtual ~ServoDriver() override;

  void setPWM(int servoNumber, int on, int off) override;
};