#pragma once
#include "system_base.hpp"
#include <ces/components.hpp>
//#include "object_handle.hpp"
#include <ces/update_state.hpp>

namespace world
{
  struct RenderSystem : public SystemBase
  {
    RenderSystem();
    bool Init();
    void Tick(const UpdateState& state);

    virtual void AddEntity(const Entity* entity);

    struct SystemEntity
    {
      PositionComponent* pos;
      RenderComponent* render;
    };

    vector<SystemEntity> entities;

    //GLuint vaoHandle;
    //GLuint vboHandle;
    //GLuint elementsHandle;
    //ObjectHandle textureHandle;
    
    //int glShaderProgram = 0;
    //int attribLocationTex = 0;
    //int attribLocationProjMtx = 0;
    //int attribLocationPosition = 0;
    //int attribLocationUV = 0;
    //int attribLocationColor = 0;
  };
  
  extern RenderSystem g_RenderSystem;
}