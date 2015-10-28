#include "input.hpp"
//#include "logging.hpp"
#include <ces/entity.hpp>
#include <ces/update_state.hpp>
//#include "sprite_manager.hpp"
//#include "global.hpp"

using namespace world;

InputSystem world::g_InputSystem;

//------------------------------------------------------------------------------
InputSystem::InputSystem()
  : SystemBase(CMPosition | CMInput)
{

}

//------------------------------------------------------------------------------
void InputSystem::AddEntity(const Entity* entity)
{
  entities.push_back(SystemEntity{
    (PositionComponent*)entity->GetComponent(CMPosition),
    (InputComponent*)entity->GetComponent(CMInput)});
}

//------------------------------------------------------------------------------
bool InputSystem::Init()
{
  return true;
}

//------------------------------------------------------------------------------
void InputSystem::Tick(const UpdateState& state)
{
  //ImGuiIO& io = ImGui::GetIO();
  //for (SystemEntity& e : entities)
  //{
  //  if (io.KeysDown['A'])
  //  {
  //    e.pos->pos.x -= 1;
  //  }
  //  else if (io.KeysDown['D'])
  //  {
  //    e.pos->pos.x += 1;
  //  }
  //  else if (io.KeysDown[GLFW_KEY_SPACE])
  //  {
  //    printf("space\n");
  //    PositionComponent* pos = e.pos;
  //    ::AddEntity(EntityBuilder(Entity::Type::Bullet)
  //              .AddComponent<PositionComponent>(pos->pos)
  //              .AddComponent<PhysicsComponent>(Vector2{0, -150}, Vector2{0,0})
  //              .AddComponent<RenderComponent>(SPRITE_MANAGER.GetSpriteIndex("laserGreen02"))
  //              .Build());
  //  }
  //}
}
