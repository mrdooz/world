#pragma once
#include "system_base.hpp"

namespace world
{
  struct UpdateState;
  struct PositionComponent;
  struct PhysicsComponent;

  struct PhysicsSystem : public SystemBase
  {
    PhysicsSystem();
    bool Init();
    void Tick(const UpdateState& state);

    virtual void AddEntity(const Entity* entity);

    struct SystemEntity
    {
      PositionComponent* pos;
      PhysicsComponent* physics;
    };

    vector<SystemEntity> entities;
  };

  extern PhysicsSystem g_PhysicsSystem;
}
