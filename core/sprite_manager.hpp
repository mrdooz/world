#pragma once
#include <lib/tano_math.hpp>
#include <core/object_handle.hpp>

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
  };

  extern SpriteManager* g_SpriteManager;
}