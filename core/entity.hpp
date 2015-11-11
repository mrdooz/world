#pragma once

namespace world
{
  struct Entity
  {
    enum Type
    {
      Player,
      Enemy,
      Bullet,
    };

    u32 type;
    u32 id;
  };

  struct EntityBuilder
  {
    static Entity Build(Entity::Type type)
    {
      return Entity{type, _nextId++};
    }

    static u32 _nextId;
  };
}
