#pragma once
#include <lib/tano_math.hpp>
#include <core/object_handle.hpp>
#include <core/gpu_objects.hpp>
#include <shaders/out/common.texture_psrendertexture.cbuffers.hpp>

//#include "types.hpp"
//#include "object_handle.hpp"

namespace world
{
  struct SpriteSheet
  {
    struct Sprite
    {
      string name;
      string sub;
      vec2 pos;
      vec2 size;
    };

    string filename;
    vec2 size;

    vector<Sprite> sprites;
  };

  struct SpriteManager
  {
    static const u16 INVALID_SPRITE = ~0;
    static bool Create();
    static bool Destroy();

    ObjectHandle LoadSpriteSheet(const char* filename);

    u16 AddSprite(const string& name,
        const string& sub,
        const vec2& uvTopLeft,
        const vec2& uvBottomRight,
        const vec2& size,
        ObjectHandle handle);

    void RenderSprite(ObjectHandle spriteSheet, int spriteId, int x, int y);
    void RenderSprites(ObjectHandle spriteSheet, const int* spriteIds, const vec2i* pos, int cnt);

    //void CopyOut(const string& name, const vec2& pos, vector<SpriteVtx>* verts, vector<u32>* indices);
    //void CopyOut(u16 idx, const vec2& pos, vector<SpriteVtx>* verts, vector<u32>* indices);
    u16 GetSpriteIndex(const string& name);

    bool Init();

    struct Sprite
    {
      vec2 uvTopLeft;
      vec2 uvBottomRight;
      vec2 size;
      ObjectHandle handle;
      u16 idx;
    };

    //void CopyOut(const Sprite* sprite, const vec2& pos, vector<SpriteVtx>* verts, vector<u32>* indices);

    unordered_map<u32, SpriteSheet> _spriteSheets;
    unordered_map<string, Sprite*> _spritesByName;
    vector<Sprite*> _sprites;

    ConstantBufferBundle<void, cb::CommonTextureP> _cbRenderTexture;
    GpuBundle _renderTextureBundle;
    GraphicsContext* _ctx = nullptr;
  };

  extern SpriteManager* g_SpriteManager;
}