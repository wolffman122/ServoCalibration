#pragma once

#include "Generator.h"
#include "IServoDriver.h"

class FanGenerator : public Generator
{

public:
  FanGenerator(IServoDriver &servoDriver, WebSocketsServer *pWebSocket, int *servoPositions, int *servoMinimums, int *servoMaximums);

  virtual void Update() override;
};