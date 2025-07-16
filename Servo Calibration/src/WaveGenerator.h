#pragma once

#include "IServoDriver.h"
#include "Generator.h"

class WaveGenerator : public Generator
{

public:
  WaveGenerator(IServoDriver &servoDriver, WebSocketsServer *pWebSocket, int *servoPositions, int *servoMinimums, int *servoMaximums);
  virtual ~WaveGenerator() override = default;
  virtual void Update() override;
};
