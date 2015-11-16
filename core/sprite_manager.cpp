#include "sprite_manager.hpp"
#include "resource_manager.hpp"
#include <lib/parse_base.hpp>
#include <lib/input_buffer.hpp>
#include <lib/error.hpp>
#include <lib/init_sequence.hpp>
#include <lib/mesh_utils.hpp>
#include <lib/string_utils.hpp>
#include <core/vertex_types.hpp>
#include <core/graphics_context.hpp>
#include <core/entity.hpp>

using namespace world;
using namespace world::parser;

static const int MAX_SPRITES_PER_BATCH = 32 * 1024;
static const float PIXELS_PER_METER = 16;

//------------------------------------------------------------------------------
SpriteManager* world::g_SpriteManager = nullptr;

//------------------------------------------------------------------------------
TmxLevel::TmxLevel()
  : world(b2Vec2(0.0f, -1.0f))
{
}

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

  QueryPerformanceFrequency(&_timerFrequency);

  // clang-format off
  INIT(_renderTextureBundle.Create(BundleOptions()
    .DepthStencilDesc(depthDescDepthDisabled)
    .RasterizerDesc(rasterizeDescCullNone)
    .VertexShader("shaders/out/sprite", "VsRenderTexture")
    .PixelShader("shaders/out/sprite", "PsRenderTexture")
    .InputElement(CD3D11_INPUT_ELEMENT_DESC("POSITION", DXGI_FORMAT_R32G32B32_FLOAT))
    .InputElement(CD3D11_INPUT_ELEMENT_DESC("TEXCOORD", DXGI_FORMAT_R32G32_FLOAT))
    .StaticIb(GenerateQuadIndices(MAX_SPRITES_PER_BATCH))
    .DynamicVb(4 * MAX_SPRITES_PER_BATCH, sizeof(PosTex))));
  // clang-format on

  INIT_FATAL(_cbRenderTexture.Create());

  END_INIT_SEQUENCE();
}

//------------------------------------------------------------------------------
void SpriteManager::AddEntity(const Entity* e, const vec2& pos, u16 sprite)
{
  assert(_entityCount < MAX_ENTITIES);
  _entityPos[_entityCount] = pos;
  _entitySprite[_entityCount] = sprite;
  _entityMap[e->id] = _entityCount;
  _entityCount++;
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
//------------------------------------------------------------------------------
template<typename T, typename TOrg = T, typename U>
bool CheckedGet(const U& m, const string& key, T* res)
{
  auto it = m.find(key);
  if (it == m.end())
  {
    LOG_WARN("Unable to find property: ", key);
    return false;
  }

  // perform a cast, as json doesn't support int types, only doubles
  *res = (T)it->second.get<TOrg>();

  return true;
}

//------------------------------------------------------------------------------
bool SpriteManager::LoadSpriteLayer(const picojson::object& layerObj)
{
  _tmxLevel.layers.push_back(TmxLayer());
  TmxLayer& tmxLayer = _tmxLevel.layers.back();

  if (
    !CheckedGet(layerObj, "name", &tmxLayer.name) ||
    !CheckedGet<int, double>(layerObj, "x", &tmxLayer.x) ||
    !CheckedGet<int, double>(layerObj, "y", &tmxLayer.y) ||
    !CheckedGet<int, double>(layerObj, "width", &tmxLayer.width) ||
    !CheckedGet<int, double>(layerObj, "height", &tmxLayer.height))
  {
    return false;
  }

  picojson::array data;
  if (!CheckedGet(layerObj, "data", &data))
    return false;

  tmxLayer.tiles.reserve(data.size());
  for (auto& d : data)
  {
    tmxLayer.tiles.push_back((int)d.get<double>());
  }

  return true;
}

//------------------------------------------------------------------------------
static b2Vec2 ScreenToBox2d(float x, float y, float zeroLevel)
{
  // apply the zero level, and scale
  // nb: this also flips the y coordinate axis, from 0 top left, to 0 center
  return b2Vec2(x / PIXELS_PER_METER, (zeroLevel - y) / PIXELS_PER_METER);
}

//------------------------------------------------------------------------------
bool SpriteManager::LoadCollisionLayer(const picojson::object& layerObj)
{
  picojson::array objects;
  if (!CheckedGet(layerObj, "objects", &objects))
    return false;

  int bbWidth, bbHeight;
  g_Graphics->GetBackBufferSize(&bbWidth, &bbHeight);

  for (auto& obj : objects)
  {
    picojson::object cur = obj.get<picojson::object>();
    // determine the collision object type
    bool ellipse;
    if (CheckedGet(cur, "ellipse", &ellipse) && ellipse)
    {
      int a = 10;
    }
    else if (cur.count("polygon"))
    {
      int a = 10;
    }
    else if (cur.count("polyline"))
    {
      vector<b2Vec2> points;
      picojson::array polylineObj = cur.find("polyline")->second.get<picojson::array>();
      for (auto& pt : polylineObj)
      {
        picojson::object ptObj = pt.get<picojson::object>();
        float x, y;
        if (
          !CheckedGet<float, double>(ptObj, "x", &x) ||
          !CheckedGet<float, double>(ptObj, "y", &y))
        {
          return false;
        }

        points.push_back(ScreenToBox2d(x, y, _tmxLevel.zeroLevel));
      }

      b2ChainShape chain;
      chain.CreateChain(points.data(), (int)points.size());

      b2BodyDef bodyDef;
      b2Body* body = _tmxLevel.world.CreateBody(&bodyDef);
      body->CreateFixture(&chain, 0.0f);
    }
    else
    {
      // if nothing else is set, then the shape is a rectangle
      float x ,y;
      float width, height;
      float rotation;

      if (
        !CheckedGet<float, double>(cur, "x", &x) ||
        !CheckedGet<float, double>(cur, "y", &y) ||
        !CheckedGet<float, double>(cur, "width", &width) ||
        !CheckedGet<float, double>(cur, "height", &height) ||
        !CheckedGet<float, double>(cur, "rotation", &rotation))
      {
        return false;
      }

      b2BodyDef bodyDef;
      b2Vec2 pos = ScreenToBox2d(x + width / 2.f, y + height / 2.f, (float)_tmxLevel.zeroLevel);
      bodyDef.position.Set(pos.x, pos.y);
      b2Body* body = _tmxLevel.world.CreateBody(&bodyDef);

      b2PolygonShape box;

      // The extents are the half-widths of the box.
      box.SetAsBox(width/ (2 * PIXELS_PER_METER), height / (2 * PIXELS_PER_METER));

      body->CreateFixture(&box, 0.0f);
    }
  }

  return true;
}

//------------------------------------------------------------------------------
bool SpriteManager::LoadTmx(const char* filename)
{
  _tmxTexture = g_ResourceManager->LoadTexture("gfx/TinyPlatformQuestTiles.png");

  vector<char> ss;
  if (!g_ResourceManager->LoadFile("tmx/level1.json", &ss))
    return false;

  picojson::value res;
  string err = picojson::parse(res, ss.begin(), ss.end());
  if (!err.empty())
  {
    LOG_WARN("Error loading tmx: ", filename, "error: ", err);
    return false;
  }

  const auto& levelObj = res.get<picojson::object>();

  if (
    !CheckedGet<int, double>(levelObj, "width", &_tmxLevel.width) ||
    !CheckedGet<int, double>(levelObj, "height", &_tmxLevel.height) ||
    !CheckedGet<int, double>(levelObj, "tilewidth", &_tmxLevel.tileWidth) ||
    !CheckedGet<int, double>(levelObj, "tileheight", &_tmxLevel.tileHeight))
  {
    return false;
  }

  if (levelObj.count("properties"))
  {
    // read the zero level
    const auto& propertiesObj = levelObj.find("properties")->second.get<picojson::object>();
    string zeroLevel;
    if (!CheckedGet(propertiesObj, "zerolevel", &zeroLevel))
    {
      return false;
    }
    _tmxLevel.zeroLevel = (float)atof(zeroLevel.c_str()) * _tmxLevel.tileHeight;
  }

  picojson::array layers;
  if (!CheckedGet(levelObj, "layers", &layers))
    return false;

  for (auto& layer : layers)
  {
    const picojson::object& layerObj = layer.get<picojson::object>();

    string name;
    if (!CheckedGet(layerObj, "name", &name))
      return false;

    transform(name.begin(), name.end(), name.begin(), tolower);
    if (StartsWith(name, "collision"))
    {
      if (!LoadCollisionLayer(layerObj))
        return false;
    }
    else
    {
      if (!LoadSpriteLayer(layerObj))
        return false;
    }
  }

  picojson::array tilesets;
  if (!CheckedGet(levelObj ,"tilesets", &tilesets))
    return false;

  for (auto& tileset : tilesets)
  {
    _tmxLevel.tilesets.push_back(TmxTileset());
    TmxTileset& tmxTileset = _tmxLevel.tilesets.back();

    const picojson::object& tilesetObj = tileset.get<picojson::object>();

    if (
      !CheckedGet(tilesetObj, "name", &tmxTileset.name) ||
      !CheckedGet(tilesetObj, "image", &tmxTileset.image) ||
      !CheckedGet<int, double>(tilesetObj, "firstgid", &tmxTileset.firstGid) ||
      !CheckedGet<int, double>(tilesetObj, "imagewidth", &tmxTileset.imageWidth) ||
      !CheckedGet<int, double>(tilesetObj, "imageheight", &tmxTileset.imageHeight) ||
      !CheckedGet<int, double>(tilesetObj, "margin", &tmxTileset.margin) ||
      !CheckedGet<int, double>(tilesetObj, "spacing", &tmxTileset.spacing) ||
      !CheckedGet<int, double>(tilesetObj, "tilecount", &tmxTileset.tileCount) ||
      !CheckedGet<int, double>(tilesetObj, "tilewidth", &tmxTileset.tileWidth) ||
      !CheckedGet<int, double>(tilesetObj, "tileheight", &tmxTileset.tileHeight))
    {
      return false;
    }
  }

  {
    static b2BodyDef bodyDef;
    bodyDef.type = b2_dynamicBody;
    bodyDef.position.Set(12, 20);
    _dynamicBody = _tmxLevel.world.CreateBody(&bodyDef);

    // Define another box shape for our dynamic body.
    static b2PolygonShape dynamicBox;
    dynamicBox.SetAsBox(1.0f, 1.0f);

    // Define the dynamic body fixture.
    b2FixtureDef fixtureDef;
    fixtureDef.shape = &dynamicBox;

    // Set the box density to be non-zero, so it will be dynamic.
    fixtureDef.density = 1.0f;

    // Override the default friction.
    fixtureDef.friction = 0.3f;

    // Add the shape to the body.
    _dynamicBody->CreateFixture(&fixtureDef);
  }

  return true;
}

//------------------------------------------------------------------------------
void SpriteManager::Tick()
{
  LARGE_INTEGER tmp;
  QueryPerformanceCounter(&tmp);
  double now = (double)tmp.QuadPart / _timerFrequency.QuadPart;
  double delta = 0;
  if (_lastTick > 0)
  {
    delta = now - _lastTick;
  }
  _lastTick = now;

  double UPDATE_FREQUENCY = 60;
  double UPDATE_INTERVAL = 1.0 / UPDATE_FREQUENCY;

  _updatedAcc += delta;
  int numTicks = (int)(_updatedAcc / UPDATE_INTERVAL);
  _updatedAcc -= numTicks * UPDATE_INTERVAL;

  int velocityIterations = 6;
  int positionIterations = 2;

  for (int i = 0; i < numTicks; ++i)
  {
    _tmxLevel.world.Step((float)UPDATE_INTERVAL, velocityIterations, positionIterations);
  }

  for (b2Contact* c = _tmxLevel.world.GetContactList(); c; c = c->GetNext())
  {
    int a = 10;
    // process c
  }
}

//------------------------------------------------------------------------------
void SpriteManager::Render()
{
  GraphicsContext* ctx = g_Graphics->GetGraphicsContext();

  int bbWidth, bbHeight;
  g_Graphics->GetBackBufferSize(&bbWidth, &bbHeight);

  int numTris = 0;
  {
    ObjectHandle h = _renderTextureBundle.objects._vb;
    PosTex* vtx = ctx->MapWriteDiscard<PosTex>(h);

    float y = (float)_tmxLevel.zeroLevel;
    float yInc = 32;
    float xInc = 32;
    int idx = 0;

    float sx = (float)_tmxLevel.tilesets[0].tileWidth / _tmxLevel.tilesets[0].imageWidth;
    float sy = (float)_tmxLevel.tilesets[0].tileHeight / _tmxLevel.tilesets[0].imageHeight;

    for (int i = 0; i < _tmxLevel.layers[0].height; ++i)
    {
      //float x = -bbWidth / 2.f;
      float x = 0;
      for (int j = 0; j < _tmxLevel.layers[0].width; ++j)
      {
        int spriteId = _tmxLevel.layers[0].tiles[idx];
        if (spriteId != 0)
        {
          spriteId -= 1;

          // 0, 1, 3
          // 3, 1, 2

          // 0, 1
          // 3, 2
          float z = 0.5f;

          int ww = _tmxLevel.tilesets[0].imageWidth / _tmxLevel.tilesets[0].tileWidth;
          int spriteX = spriteId % ww;
          int spriteY = spriteId / ww;

          vtx[0] = PosTex{vec3{x, y, z}, vec2{sx * spriteX, sy * spriteY}};
          vtx[1] = PosTex{vec3{x + xInc, y, z}, vec2{sx * spriteX + sx, sy * spriteY}};
          vtx[2] = PosTex{vec3{x + xInc, y - yInc, z}, vec2{sx * spriteX + sx, sy * spriteY + sy}};
          vtx[3] = PosTex{vec3{x, y - yInc, z}, vec2{sx * spriteX, sy * spriteY + sy}};

          vtx += 4;
          numTris += 2;
        }
        x += xInc;
        idx++;
      }

      y -= yInc;
    }

    {
      // HACK HACK!
      b2Vec2 p = _dynamicBody->GetPosition();
      float x = p.x * PIXELS_PER_METER;
      float y = p.y * PIXELS_PER_METER;
      float z = 0.5f;

      int spriteX = 1;
      int spriteY = 0;
      vtx[0] = PosTex{ vec3{ x, y, z }, vec2{ sx * spriteX, sy * spriteY } };
      vtx[1] = PosTex{ vec3{ x + xInc, y, z }, vec2{ sx * spriteX + sx, sy * spriteY } };
      vtx[2] = PosTex{ vec3{ x + xInc, y - yInc, z }, vec2{ sx * spriteX + sx, sy * spriteY + sy } };
      vtx[3] = PosTex{ vec3{ x, y - yInc, z }, vec2{ sx * spriteX, sy * spriteY + sy } };

      numTris += 2;
    }

    ctx->Unmap(h);
  }

  mat4x4 view = MatrixLookAtLH(vec3(0, bbHeight/2.f, -1), vec3(0, bbHeight/2.f, 0), vec3(0, 1, 0));
  mat4x4 proj = MatrixOrthoLH((float)bbWidth, (float)bbHeight, 0, 10);

  mat4x4 viewProj = view * proj;

  _cbRenderTexture.vs0.viewProj = Transpose(viewProj);
  _cbRenderTexture.Set(ctx, 0);
  ctx->SetBundleWithSamplers(_renderTextureBundle, ShaderType::PixelShader);
  ctx->SetShaderResource(_tmxTexture);
  ctx->DrawIndexed(6 * numTris, 0, 0);
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
    
    //vtx[0] = PosTex{ pos.x, pos.y, 0, 0, 0 };
    //vtx[0] = PosTex{ pos.x, pos.y, 0, 0, 0 };
    //vtx[0] = PosTex{ pos.x, pos.y, 0, 0, 0 };
    //vtx[0] = PosTex{ pos.x, pos.y, 0, 0, 0 };
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
