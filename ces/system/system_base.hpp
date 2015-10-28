#pragma once

namespace world
{
  struct Entity;
  struct UpdateState;

  struct SystemBase
  {
    SystemBase(u64 componentMask) : componentMask(componentMask) {}
    virtual ~SystemBase() {};

    virtual void AddEntity(const Entity* entity) = 0;
    virtual void Tick(const UpdateState& state) {}

    u64 componentMask = 0;
  };
}