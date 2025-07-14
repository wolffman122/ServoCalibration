#include "ServoDriver.h"

ServoDriver::ServoDriver()
{
  m_Pca9685 = Adafruit_PWMServoDriver(0x40);

  m_Pca9685.begin();

  Serial.println("PCA9685 initialized");
  m_Pca9685.setPWMFreq(50);
  Serial.println("PCA9685 frequency set to 50Hz");
}

ServoDriver::~ServoDriver()
{
  // Destructor implementation
}

void ServoDriver::setPWM(int servoNumber, int on, int off)
{
  m_Pca9685.setPWM(servoNumber, on, off);
}