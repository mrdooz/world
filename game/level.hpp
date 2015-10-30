#pragma once
#include <core/object_handle.hpp>
#include <lib/tano_math.hpp>

namespace world
{
  //------------------------------------------------------------------------------
  struct BackgroundPage
  {
    int x, y;
    int w, h;

    void RenderToRenderTarget(ObjectHandle renderTarget);

    vector<vec2i> walls;
  };

  //------------------------------------------------------------------------------
  struct Level
  {
    bool Load(const char* filename);

    vector<BackgroundPage> pages;
  };
}