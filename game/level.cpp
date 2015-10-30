#include "level.hpp"
#include <core/resource_manager.hpp>
#include <core/sprite_manager.hpp>
#include <lib/init_sequence.hpp>

using namespace world;

static int PAGE_SIZE = 256;

//------------------------------------------------------------------------------
bool Level::Load(const char* filename)
{
  BEGIN_INIT_SEQUENCE();

  u8* buf;
  int w, h, c;
  INIT_FATAL(g_ResourceManager->LoadImage(filename, &buf, &w, &h, &c));
  u32* buf32 = (u32*)buf;

  ObjectHandle spriteSheet;
  INIT_RESOURCE_FATAL(spriteSheet, g_SpriteManager->LoadSpriteSheet("gfx/oryx_16bit_fantasy_world_trans.sheet"));

  int pageCountX = (w + PAGE_SIZE - 1) / PAGE_SIZE;
  int pageCountY = (h + PAGE_SIZE - 1) / PAGE_SIZE;

  for (int pageY = 0; pageY < pageCountY; ++pageY)
  {
    for (int pageX = 0; pageX < pageCountX; ++pageX)
    {
      pages.push_back(BackgroundPage {pageX * PAGE_SIZE, pageY * PAGE_SIZE, PAGE_SIZE, PAGE_SIZE});
      BackgroundPage& page = pages.back();

      for (int i = 0; i < PAGE_SIZE; ++i)
      {
        for (int j = 0; j < PAGE_SIZE; ++j)
        {
          int xx = pageX * PAGE_SIZE + j;
          int yy = pageY * PAGE_SIZE + i;
          u32 cur = buf32[yy * w + xx];
          if (cur > 0xff000000)
          {
            page.walls.push_back(vec2i{j, i});
            int a = 10;
          }
        }
      }
    }
  }

  END_INIT_SEQUENCE();
}

//------------------------------------------------------------------------------
void BackgroundPage::RenderToRenderTarget(ObjectHandle renderTarget)
{
}
