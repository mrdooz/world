#include "components.hpp"
//#include "sprite_manager.hpp"
//#include "types.hpp"
#include "system/render.hpp"

using namespace world;

namespace world
{
  typedef ComponentFactory<PositionComponent> PositionComponentFactory;
  typedef ComponentFactory<RenderComponent> RenderComponentFactory;
  typedef ComponentFactory<InputComponent> InputComponentFactory;
  typedef ComponentFactory<PhysicsComponent> PhysicsComponentFactory;

  void CreateFreeList(int stride, int numElems, char* buf)
  {
    for (int i = 0; i < numElems; ++i)
    {
      u16* p = (u16*)(buf + i * stride);
      *p = (i == numElems-1) ? 0xffff : i + 1;
    }
  }
}
