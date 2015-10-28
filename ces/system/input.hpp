#pragma once
#include "system_base.hpp"

namespace world
{
  struct UpdateState;
  struct PositionComponent;
  struct InputComponent;

  struct InputSystem : public SystemBase
  {
    InputSystem();
    bool Init();
    void Tick(const UpdateState& state);

    virtual void AddEntity(const Entity* entity);

    struct SystemEntity
    {
      PositionComponent* pos;
      InputComponent* input;
    };

    vector<SystemEntity> entities;
  };

  extern InputSystem g_InputSystem;
}
