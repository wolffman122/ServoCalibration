#pragma once

class IGenerator
{
public:
  virtual ~IGenerator() = default;

  virtual void Update() = 0;
  virtual void Start() = 0;
  virtual void Stop() = 0;

  virtual bool isStarted() const = 0;
  virtual void AddServo() = 0;
  virtual void RemoveServo() = 0;
};