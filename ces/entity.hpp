#pragma once
#include "components.hpp"

namespace world
{
  struct PositionComponent;
  struct RenderComponent;

  struct Entity
  {
    enum class Type
    {
      Player,
      Enemy,
      Bullet,
    };

    Entity(Type type) : type(type) {}

    void* GetComponent(ComponentMask mask) const
    {
      for (const ComponentData& c : components)
      {
        if (c.type == mask)
          return c.ptr;
      }

      return nullptr;
    }

    struct ComponentData
    {
      void* ptr;
      ComponentMask type;
    };

    vector<ComponentData> components;

    Type type;
    u64 componentMask = 0;
  };

  struct EntityBuilder
  {
    EntityBuilder(Entity::Type type) : type(type) {}

    template <typename T, typename... Args>
    EntityBuilder& AddComponent(Args... args)
    {
      void* ptr = T::FACTORY.AllocComponent(args...);
      ComponentMask type = (ComponentMask)T::type;
      components.push_back({ptr, type });
      componentMask |= (u64)type;
      return *this;
    }

    Entity* Build()
    {
      Entity* e = new Entity(type);
      e->components = components;
      e->componentMask = componentMask;
      return e;
    }

    Entity::Type type;
    vector<Entity::ComponentData> components;
    u64 componentMask = 0;
  };
}