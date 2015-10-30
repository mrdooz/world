#include "sprite_manager.hpp"
#include "resource_manager.hpp"
#include <lib/parse_base.hpp>
#include <lib/input_buffer.hpp>
#include <lib/error.hpp>
#include <lib/init_sequence.hpp>
#include <lib/mesh_utils.hpp>
#include <core/vertex_types.hpp>

using namespace world;
using namespace world::parser;

static const int MAX_SPRITES_PER_BATCH = 32 * 1024;

//------------------------------------------------------------------------------
SpriteManager* world::g_SpriteManager = nullptr;

//------------------------------------------------------------------------------
bool SpriteManager::Create()
{
  assert(!g_SpriteManager);

  g_SpriteManager = new SpriteManager();
  return g_SpriteManager->Init();
}

//------------------------------------------------------------------------------
bool SpriteManager::Destroy()
{
  return true;
}

//------------------------------------------------------------------------------
bool SpriteManager::Init()
{
  BEGIN_INIT_SEQUENCE();

  // clang-format off
  INIT(_renderTextureBundle.Create(BundleOptions()
    .DepthStencilDesc(depthDescDepthDisabled)
    .RasterizerDesc(rasterizeDescCullNone)
    .VertexShader("shaders/out/common.texture", "VsRenderTexture")
    .PixelShader("shaders/out/common.texture", "PsRenderTexture")
    .InputElement(CD3D11_INPUT_ELEMENT_DESC("SV_POSITION", DXGI_FORMAT_R32G32B32A32_FLOAT))
    .InputElement(CD3D11_INPUT_ELEMENT_DESC("TEXCOORD", DXGI_FORMAT_R32G32_FLOAT))
    .StaticIb(GenerateQuadIndices(MAX_SPRITES_PER_BATCH))
    .DynamicVb(4 * MAX_SPRITES_PER_BATCH, sizeof(PosTex))));
  // clang-format on

  INIT_FATAL(_cbRenderTexture.Create());

  END_INIT_SEQUENCE();
}

//------------------------------------------------------------------------------
u16 SpriteManager::AddSprite(const string& name,
    const string& sub,
    const vec2& uvTopLeft,
    const vec2& uvBottomRight,
    const vec2& size,
    ObjectHandle handle)
{
  u16 id = (u16)_sprites.size();
  Sprite* sprite = new Sprite{uvTopLeft, uvBottomRight, size, handle, id};
  _spritesByName[name] = sprite;
  _sprites.push_back(sprite);
  return id;
}

#if 0
//------------------------------------------------------------------------------
void SpriteManager::CopyOut(const Sprite* sprite, const vec2& pos, vector<SpriteVtx>* verts, vector<u32>* indices)
{
  // 1--2
  // |  |
  // 0--3

  // CCW:
  // 0, 3, 1
  // 3, 2, 1

  int vertOfs = (int)verts->size();

  vec2 v0 = vec2{pos.x, pos.y + sprite->size.y};
  vec2 v1 = vec2{pos.x, pos.y};
  vec2 v2 = vec2{pos.x + sprite->size.x, pos.y};
  vec2 v3 = vec2{pos.x + sprite->size.x, pos.y + sprite->size.y};

  vec2 t0{sprite->uvTopLeft.x, sprite->uvBottomRight.y};
  vec2 t1{sprite->uvTopLeft.x, sprite->uvTopLeft.y};
  vec2 t2{sprite->uvBottomRight.x, sprite->uvTopLeft.y};
  vec2 t3{sprite->uvBottomRight.x, sprite->uvBottomRight.y};

  u32 c = 0xffffffff;

  verts->push_back(SpriteVtx{v0, t0, c});
  verts->push_back(SpriteVtx{v1, t1, c});
  verts->push_back(SpriteVtx{v2, t2, c});
  verts->push_back(SpriteVtx{v3, t3, c});

  indices->push_back(vertOfs + 0);
  indices->push_back(vertOfs + 3);
  indices->push_back(vertOfs + 1);

  indices->push_back(vertOfs + 3);
  indices->push_back(vertOfs + 2);
  indices->push_back(vertOfs + 1);
}

//------------------------------------------------------------------------------
void SpriteManager::CopyOut(const string& name, const vec2& pos, vector<SpriteVtx>* verts, vector<u32>* indices)
{
  auto it = _spritesByName.find(name);
  if (it == _spritesByName.end())
    return;

  CopyOut(it->second, pos, verts, indices);
}

//------------------------------------------------------------------------------
void SpriteManager::CopyOut(u16 idx, const vec2& pos, vector<SpriteVtx>* verts, vector<u32>* indices)
{
  assert(idx < _sprites.size());
  assert(_sprites[idx]);

  CopyOut(_sprites[idx], pos, verts, indices);
}
#endif

//------------------------------------------------------------------------------
u16 SpriteManager::GetSpriteIndex(const string& name)
{
  auto it = _spritesByName.find(name);
  if (it == _spritesByName.end())
    return INVALID_SPRITE;

  return it->second->idx;
}

//------------------------------------------------------------------------------
bool SkipToken(InputBuffer& buf, char token)
{
  buf.SkipWhitespace();
  if (!buf.Expect(token))
    return false;
  buf.SkipWhitespace();
  return true;
}

//------------------------------------------------------------------------------
struct KeywordParser
{
  KeywordParser(InputBuffer& input) : input(input) {}
  bool Run()
  {
    input.SkipWhitespace();
    while (!input.Eof())
    {
      string keyword = ParseIdentifier(input);
      input.SkipWhitespace();

      auto it = evals.find(keyword);
      if (it == evals.end())
      {
        LOG_ERROR("Unknown keyword found: ", keyword);
        return false;
      }

      try
      {
        it->second();
      }
      catch (ParseException)
      {
        return false;
      }
      input.SkipWhitespace();
    }
    return true;
  }

  void AddEval(const string& keyword, const function<void()>& fn) { evals[keyword] = fn; }

  unordered_map<string, function<void()>> evals;
  InputBuffer& input;
};

//------------------------------------------------------------------------------
static bool ParseSpriteSheet(InputBuffer& buf, SpriteSheet* sheet)
{
  KeywordParser k(buf);
  k.AddEval("filename",
      [&]()
      {
        sheet->filename = ParseString(buf);
        buf.Expect(';');
      });
  k.AddEval("size",
      [&]()
      {
        sheet->size = ParseVec2(buf);
        buf.Expect(';');
      });

  return k.Run();
}

//------------------------------------------------------------------------------
static void ParseSingleSprite(InputBuffer& buf, vector<SpriteSheet::Sprite>* sprites)
{
  string name;
  string sub;
  vec2 offset = vec2{0, 0};
  vec2 size = vec2{0, 0};
  int repeat = 1;
  int spacing = 0;

  KeywordParser k(buf);
  k.AddEval("name",
      [&]()
      {
        name = ParseString(buf);
        buf.Expect(';');
      });
  k.AddEval("sub",
      [&]()
      {
        sub = ParseIdentifier(buf, false);
        buf.Expect(';');
      });
  k.AddEval("offset",
      [&]()
      {
        offset = ParseVec2(buf);
        buf.Expect(';');
      });
  k.AddEval("size",
      [&]()
      {
        size = ParseVec2(buf);
        buf.Expect(';');
      });
  k.AddEval("repeat",
      [&]()
      {
        repeat = ParseInt(buf);
        buf.Expect(';');
      });
  k.AddEval("spacing",
      [&]()
      {
        spacing = ParseInt(buf);
        buf.Expect(';');
      });

  if (!k.Run())
    return;

  // create the sprite settings from the input
  vec2 pos = offset;
  for (int i = 0; i < repeat; ++i)
  {
    sprites->push_back(SpriteSheet::Sprite{name, sub, pos, size});
    pos.x += size.x + spacing;
    pos.y += size.y + spacing;
  }
}

//------------------------------------------------------------------------------
ObjectHandle SpriteManager::LoadSpriteSheet(const char* filename)
{
  vector<char> buf;
  if (!g_ResourceManager->LoadFile(filename, &buf))
    return ObjectHandle();

  SpriteSheet sheet;
  InputBuffer input(buf);
  KeywordParser k(input);

  k.AddEval("sprite",
      [&]()
      {
        {
          InnerScope inner(input, "{}");
          ParseSingleSprite(inner.inner, &sheet.sprites);
        }
        input.Expect(';');
      });

  k.AddEval("sheet",
      [&]()
      {
        {
          InnerScope inner(input, "{}");
          ParseSpriteSheet(inner.inner, &sheet);
        }
        input.Expect(';');
      });

  if (!k.Run())
    return ObjectHandle();

  ObjectHandle res = ObjectHandle(ObjectHandle::kSpriteSheet, (int)_spriteSheets.size());
  _spriteSheets[res.id()] = sheet;
  return res;
}

//------------------------------------------------------------------------------
void SpriteManager::RenderSprites(
    ObjectHandle spriteSheet, const int* spriteIds, const vec2i* pos, int cnt)
{
  auto it = _spriteSheets.find(spriteSheet.id());
  if (it == _spriteSheets.end())
  {
    LOG_ERROR("Invalid sprite sheet id: ", spriteSheet.id());
    return;
  }

  const SpriteSheet& sheet = it->second;

  ObjectHandle h = _renderTextureBundle.objects._vb;
  PosTex* vtx = _ctx->MapWriteDiscard<PosTex>(h);

  for (int i = 0; i < cnt;  ++i)
  {
    int spriteId = spriteIds[i];
    if (spriteId >= sheet.sprites.size())
    {
      LOG_ERROR("Invalid sprite id: ", spriteId);
      return;
    }

    const SpriteSheet::Sprite& sprite = sheet.sprites[spriteId];

    // 0--1
    // 2--3
    
    vtx[0] = PosTex{ pos.x, pos.y, 0, 0, 0 };
    vtx[0] = PosTex{ pos.x, pos.y, 0, 0, 0 };
    vtx[0] = PosTex{ pos.x, pos.y, 0, 0, 0 };
    vtx[0] = PosTex{ pos.x, pos.y, 0, 0, 0 };
  }
  _ctx->Unmap(h);
}

//------------------------------------------------------------------------------
void SpriteManager::RenderSprite(ObjectHandle spriteSheet, int spriteId, int x, int y)
{
  auto it = _spriteSheets.find(spriteSheet.id());
  if (it == _spriteSheets.end())
  {
    LOG_ERROR("Invalid sprite sheet id: ", spriteSheet.id());
    return;
  }

  const SpriteSheet& sheet = it->second;
  if (spriteId >= sheet.sprites.size())
  {
    LOG_ERROR("Invalid sprite id: ", spriteId);
    return;
  }
}
