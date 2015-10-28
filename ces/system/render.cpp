#include "render.hpp"
//#include "parse_sprite.hpp"
//#include "graphics.hpp"
//#include "logging.hpp"
//#include "sprite_manager.hpp"
#include <ces/entity.hpp>

using namespace world;

RenderSystem world::g_RenderSystem;

//------------------------------------------------------------------------------
RenderSystem::RenderSystem()
  : SystemBase(CMPosition | CMRender)
{
}

//------------------------------------------------------------------------------
void RenderSystem::AddEntity(const Entity* entity)
{
  entities.push_back(SystemEntity{
    (PositionComponent*)entity->GetComponent(CMPosition),
    (RenderComponent*)entity->GetComponent(CMRender)});
}

//------------------------------------------------------------------------------
bool RenderSystem::Init()
{
//  SpriteSheetSettings sheet;
//  ParseSpriteSheet("gfx/space_sheet1.txt", &sheet);
//  textureHandle = GRAPHICS.CreateSpriteSheet(sheet);
//  ObjectHandle shaderProgram = GRAPHICS.LoadShaderProgram("shaders/sprite.vs", "shaders/sprite.fs");
//  glShaderProgram = GRAPHICS._resShaderPrograms.Get(shaderProgram).glHandle;
//
//  GL_CHECK(attribLocationTex = glGetUniformLocation(glShaderProgram, "Texture"));
//  GL_CHECK(attribLocationProjMtx = glGetUniformLocation(glShaderProgram, "ProjMtx"));
//  GL_CHECK(attribLocationPosition = glGetAttribLocation(glShaderProgram, "Position"));
//  GL_CHECK(attribLocationUV = glGetAttribLocation(glShaderProgram, "UV"));
//  GL_CHECK(attribLocationColor = glGetAttribLocation(glShaderProgram, "Color"));
//
//  glGenBuffers(1, &vboHandle);
//  glGenBuffers(1, &elementsHandle);
//
//  glGenVertexArrays(1, &vaoHandle);
//  glBindVertexArray(vaoHandle);
//  glBindBuffer(GL_ARRAY_BUFFER, vboHandle);
//  glEnableVertexAttribArray(attribLocationPosition);
//  glEnableVertexAttribArray(attribLocationUV);
//  glEnableVertexAttribArray(attribLocationColor);
//
//#define OFFSETOF(TYPE, ELEMENT) ((size_t)&(((TYPE *)0)->ELEMENT))
//  glVertexAttribPointer(attribLocationPosition, 2, GL_FLOAT, GL_FALSE, sizeof(SpriteVtx), (GLvoid*)OFFSETOF(SpriteVtx, pos));
//  glVertexAttribPointer(attribLocationUV, 2, GL_FLOAT, GL_FALSE, sizeof(SpriteVtx), (GLvoid*)OFFSETOF(SpriteVtx, uv));
//  glVertexAttribPointer(attribLocationColor, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(SpriteVtx), (GLvoid*)OFFSETOF(SpriteVtx, col));
//#undef OFFSETOF
//
//  glBindVertexArray(0);
//  glBindBuffer(GL_ARRAY_BUFFER, 0);

  return true;
}

//------------------------------------------------------------------------------
void RenderSystem::Tick(const UpdateState& state)
{
  //const float w = ImGui::GetIO().DisplaySize.x;
  //const float h = ImGui::GetIO().DisplaySize.y;
  //const float orthoProjection[4][4] =
  //{
  //  {  2.0f/w,  0.0f,			 0.0f,		0.0f },
  //  {  0.0f,		2.0f/-h,	 0.0f,		0.0f },
  //  {  0.0f,		0.0f,			-1.0f,		0.0f },
  //  { -1.0f,		1.0f,			 0.0f,		1.0f },
  //};

  //glDisable(GL_CULL_FACE);
  //glDisable(GL_DEPTH_TEST);
  //glActiveTexture(GL_TEXTURE0);

  //glUseProgram(glShaderProgram);
  //glUniform1i(attribLocationTex, 0);
  //glUniformMatrix4fv(attribLocationProjMtx, 1, GL_FALSE, &orthoProjection[0][0]);

  //glBindVertexArray(vaoHandle);

  //vector<SpriteVtx> verts;
  //vector<u32> indices;

  //// copy out all the active components
  //for (SystemEntity& e : entities)
  //{
  //  SPRITE_MANAGER.CopyOut(e.render->spriteIdx, e.pos->pos, &verts, &indices);
  //}

  //GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, vboHandle));
  //GL_CHECK(glBufferData(GL_ARRAY_BUFFER, (GLsizeiptr)verts.size() * sizeof(SpriteVtx), (GLvoid*)verts.data(), GL_STREAM_DRAW));

  //GL_CHECK(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementsHandle));
  //GL_CHECK(glBufferData(GL_ELEMENT_ARRAY_BUFFER, (GLsizeiptr)indices.size() * sizeof(u32), (GLvoid*)indices.data(), GL_STREAM_DRAW));

  //GL_CHECK(glBindTexture(GL_TEXTURE_2D, GRAPHICS._resTextures.Get(textureHandle).glHandle));
  //GL_CHECK(glDrawElements(GL_TRIANGLES, (GLsizei)indices.size(), GL_UNSIGNED_INT, 0));
}
