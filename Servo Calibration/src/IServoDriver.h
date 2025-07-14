#pragma once


class IServoDriver
{
public:
  virtual ~IServoDriver() = default;

  virtual void setPWM(int servoNumber, int on, int off) = 0;
};