#pragma once

#include "IServoDriver.h"
#include <WebSocketsServer.h>
#include "IGenerator.h"

class Generator : public IGenerator
{
protected:
  IServoDriver &m_ServoDriver;
  WebSocketsServer *m_pWebSocket;
  bool m_Started = false;
  int m_Position = 0;
  int m_Direction = 1; // 1 = increasing, -1 = decreasing
  unsigned long m_LastUpdate = 0;
  const int m_Step = 2;                // degrees per update
  const unsigned long m_Interval = 20; // ms between updates
  int m_NumberOfServos = 1;            // Number of servos in the wave
  int *m_ServoMinimums = nullptr;
  int *m_ServoMaximums = nullptr;

public:
  Generator(IServoDriver &servoDriver, WebSocketsServer *pWebSocket, int *servoMinimums, int *servoMaximums);

  virtual ~Generator() override = default;

  virtual void Update() = 0;
  virtual void Start() override;
  virtual void Stop() override;

  virtual bool isStarted() const override;
  virtual void AddServo() override;
  virtual void RemoveServo() override;
};
