#pragma once
#include <lib/tano_math.hpp>

namespace world
{
  enum ComponentMask
  {
    CMPosition = 1 << 0,
    CMRender = 1 << 1,
    CMInput = 1 << 2,
    CMPhysics = 1 << 3,
  };

  void CreateFreeList(int stride, int numElems, char* buf);

  template <typename T, int MAX_NUM_COMPONENTS = 1024>
  struct ComponentFactory
  {
    ComponentFactory()
    {
      // create the free list between the components
      CreateFreeList(sizeof(T), MAX_NUM_COMPONENTS, (char*)components);
      activeComponents[0] = INVALID_SPRITE;
    }

    template <typename... Args>
    T* AllocComponent(Args... args)
    {
      // grab the next free component, and use his "next free" for the new head
      char* ptr = &components[freeListHead * sizeof(T)];
      activeComponents[numActiveComponents] = freeListHead;
      activeComponents[++numActiveComponents] = -1;
      freeListHead = *(u16*)ptr;
      T* obj = new(ptr)T{args...};
      return obj;
    }

    void FreeComponent(T* ptr)
    {
      u16 idx = ptr - (T*)components;
      *(u16*)ptr = freeListHead;
      freeListHead = idx;

      // compact the active component list
      int* obj = &activeComponents[idx];
      memmove(obj, obj + 1, (MAX_NUM_COMPONENTS-idx-1)*sizeof(int));
      numActiveComponents--;
      activeComponents[numActiveComponents] = INVALID_SPRITE;
    }

    const T& Get(int idx)
    {
      return *(T*)&components[idx*sizeof(T)];
    }

    char components[sizeof(T) * MAX_NUM_COMPONENTS];
    int activeComponents[MAX_NUM_COMPONENTS+1];
    int numActiveComponents = 0;
    u16 freeListHead = 0;
  };

  struct PositionComponent
  {
    enum { type = ComponentMask::CMPosition };
    vec2 pos;
  };

  struct RenderComponent
  {
    enum { type = ComponentMask::CMRender };
    u32 spriteIdx;
  };

  struct InputComponent
  {
    enum { type = ComponentMask::CMInput };
  };

  struct PhysicsComponent
  {
    enum { type = ComponentMask::CMPhysics };
    vec2 vel;
    vec2 acc;
  };
}
