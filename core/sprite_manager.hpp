#pragma once
#include <lib/tano_math.hpp>
#include <core/object_handle.hpp>
#include <core/gpu_objects.hpp>
#include <shaders/out/sprite_vsrendertexture.cbuffers.hpp>

//#include "types.hpp"
//#include "object_handle.hpp"

namespace world
{
  struct TmxLayer
  {
    string name;
    int x, y;
    int width, height;
    vector<u32> tiles;
  };

  struct TmxTileset
  {
    int firstGid;
    string image;
    int imageWidth, imageHeight;
    string name;
    int margin, spacing;
    int tileCount;
    int tileWidth, tileHeight;
  };

  struct TmxLevel
  {
    int width, height;
    int tileWidth, tileHeight;

    vector<TmxLayer> layers;
    vector<TmxTileset> tilesets;
  };

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
    ObjectHandle texture;
  };

  struct Entity;
  struct SpriteManager
  {
    static const u16 INVALID_SPRITE = ~0;
    static bool Create();
    static bool Destroy();

    bool LoadTmx(const char* filename);

    ObjectHandle LoadSpriteSheet(const char* filename);

    void AddEntity(const Entity* e, const vec2& pos, u16 sprite);

    u16 AddSprite(const string& name,
        const string& sub,
        const vec2& uvTopLeft,
        const vec2& uvBottomRight,
        const vec2& size,
        ObjectHandle handle);

    void RenderSprite(ObjectHandle spriteSheet, int spriteId, int x, int y);
    void RenderSprites(ObjectHandle spriteSheet, const int* spriteIds, const vec2i* pos, int cnt);

    void Render();
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

    ConstantBufferBundle<cb::SpriteV> _cbRenderTexture;
    GpuBundle _renderTextureBundle;
    GraphicsContext* _ctx = nullptr;

    unordered_map<u32, u32> _entityMap;
    enum { MAX_ENTITIES = 16 * 1024};
    vec2 _entityPos[MAX_ENTITIES];
    u16 _entitySprite[MAX_ENTITIES];
    int _entityCount = 0;

    ObjectHandle _tmxTexture;
    TmxLevel _tmxLevel;

  };

  extern SpriteManager* g_SpriteManager;
}