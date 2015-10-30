#include "mesh_utils.hpp"
#include "init_sequence.hpp"
#include "arena_allocator.hpp"

using namespace world;

namespace world
{
  //------------------------------------------------------------------------------
  vector<u32> GenerateQuadIndices(int numQuads)
  {
    vector<u32> indices(numQuads * 6);
    for (int i = 0; i < numQuads; ++i)
    {
      // 0, 1, 3
      indices[i * 6 + 0] = i * 4 + 0;
      indices[i * 6 + 1] = i * 4 + 1;
      indices[i * 6 + 2] = i * 4 + 3;

      // 3, 1, 2
      indices[i * 6 + 3] = i * 4 + 3;
      indices[i * 6 + 4] = i * 4 + 1;
      indices[i * 6 + 5] = i * 4 + 2;
    }

    return indices;
  }

  //------------------------------------------------------------------------------
  vector<u32> GenerateCubeIndices(int numCubes)
  {
    //    5----6
    //   /    /|
    //  1----2 |
    //  | 4--|-7
    //  |/   |/
    //  0----3

    vector<u32> indices(numCubes * 36);
    u32* ptr = indices.data();

    auto AddFace = [&](int i, int a, int b, int c){
      *ptr++ = i * 8 + a;
      *ptr++ = i * 8 + b;
      *ptr++ = i * 8 + c;
    };

    for (int i = 0; i < numCubes; ++i)
    {
      // Front face
      AddFace(i, 0, 1, 2);
      AddFace(i, 0, 2, 3);

      // Back face
      AddFace(i, 4, 6, 5);
      AddFace(i, 4, 7, 6);

      // Left face
      AddFace(i, 4, 5, 1);
      AddFace(i, 4, 1, 0);

      // Right face
      AddFace(i, 3, 2, 6);
      AddFace(i, 3, 6, 7);

      // Top face
      AddFace(i, 1, 5, 6);
      AddFace(i, 1, 6, 2);

      // Bottom face
      AddFace(i, 4, 0, 3);
      AddFace(i, 4, 3, 7);
    }

    return indices;
  }

  //------------------------------------------------------------------------------
  vec3* AddCube(vec3* buf, const vec3& pos, float scale)
  {
    *buf++ = pos + vec3(-scale, -scale, -scale);
    *buf++ = pos + vec3(-scale, +scale, -scale);
    *buf++ = pos + vec3(+scale, +scale, -scale);
    *buf++ = pos + vec3(+scale, -scale, -scale);
    *buf++ = pos + vec3(-scale, -scale, +scale);
    *buf++ = pos + vec3(-scale, +scale, +scale);
    *buf++ = pos + vec3(+scale, +scale, +scale);
    *buf++ = pos + vec3(+scale, -scale, +scale);
    return buf;
  }

  //------------------------------------------------------------------------------
  vector<u32> GenerateCubeIndicesFaceted(int numCubes)
  {
    //    5----6
    //   /    /|
    //  1----2 |
    //  | 4--|-7
    //  |/   |/
    //  0----3

    vector<u32> indices(numCubes * 36);
    u32* ptr = indices.data();

    auto AddFace = [&](int i, int a, int b, int c){
      *ptr++ = i + a;
      *ptr++ = i + b;
      *ptr++ = i + c;
    };

    // In a faceted cube all vertices are unique, so it's basically just
    // repeating adding the first 2 faces 6 times
    for (int i = 0; i < numCubes; ++i)
    {
      for (int j = 0; j < 6; ++j)
      {
        AddFace(i * 24 + j * 4, 0, 1, 2);
        AddFace(i * 24 + j * 4, 0, 2, 3);
      }
    }

    return indices;
  }
  //------------------------------------------------------------------------------
  static void AddFaceIndices(u32 a, u32 b, u32 c, vector<u32>* indices)
  {
    indices->push_back(a);
    indices->push_back(b);
    indices->push_back(c);
  }

  //------------------------------------------------------------------------------
  vector<u32> CreateCylinderIndices(int numRingSegments, int numHeightSegments, bool segmented)
  {
    vector<u32> indices;

    // 1--2
    // |  |
    // 0--3

    int mul = segmented ? 2 : 1;

    // TODO(magnus): hm, this vertex ordering seems off. they should grow in the
    // other direction
    for (int i = 0; i < numHeightSegments-1; ++i)
    {
      for (int j = 0; j < numRingSegments; ++j)
      {
        u32 v0 = (i * mul + 1) * numRingSegments + j;
        u32 v1 = (i * mul + 0) * numRingSegments + j;
        u32 v2 = (i * mul + 0) * numRingSegments + (j+1) % numRingSegments;
        u32 v3 = (i * mul + 1) * numRingSegments + (j+1) % numRingSegments;

        AddFaceIndices(v0, v1, v2, &indices);
        AddFaceIndices(v0, v2, v3, &indices);
      }
    }
    return indices;
  }

  //------------------------------------------------------------------------------
  vec3* AddCubeWithNormal(vec3* buf, const vec3& pos, float scale, bool singleFace)
  {
    vec3 verts[] = {
      {pos + vec3(-scale, -scale, -scale)},
      {pos + vec3(-scale, +scale, -scale)},
      {pos + vec3(+scale, +scale, -scale)},
      {pos + vec3(+scale, -scale, -scale)},
      {pos + vec3(-scale, -scale, +scale)},
      {pos + vec3(-scale, +scale, +scale)},
      {pos + vec3(+scale, +scale, +scale)},
      {pos + vec3(+scale, -scale, +scale)},
    };

    vec3 n0(0, 0, -1);
    vec3 n1(0, 0, +1);
    vec3 n2(-1, 0, 0);
    vec3 n3(+1, 0, 0);
    vec3 n4(0, +1, 0);
    vec3 n5(0, -1, 0);

    //    5----6
    //   /    /|
    //  1----2 |
    //  | 4--|-7
    //  |/   |/
    //  0----3

    auto AddFace = [&](int a, int b, int c, int d, const vec3& n){
      *buf++ = verts[a]; *buf++ = n;
      *buf++ = verts[b]; *buf++ = n;
      *buf++ = verts[c]; *buf++ = n;
      *buf++ = verts[d]; *buf++ = n;
    };

    /// Front face
    AddFace(0, 1, 2, 3, n0);

    if (singleFace)
    {
      return buf;
    }

    // Back face
    AddFace(7, 6, 5, 4, n1);

    // Left face
    AddFace(3, 2, 6, 7, n2);

    // Right face
    AddFace(4, 5, 1, 0, n3);

    // Top face
    AddFace(1, 5, 6, 2, n4);

    // Bottom face
    AddFace(4, 0, 3, 7, n5);

    return buf;
  }

  //------------------------------------------------------------------------------
  void GeneratePlaneIndices(int width, int height, int ofs, vector<u32>* out)
  {
    size_t oldSize = out->size();
    out->resize(oldSize + (width - 1) * (height - 1) * 6);
    u32* res = out->data() + oldSize;

    // 1 2
    // 0 3

    for (int i = 0; i < height - 1; ++i)
    {
      for (int j = 0; j < width - 1; ++j)
      {
        // 0, 1, 3
        res[0] = ofs + (i + 0) * width + (j + 0);
        res[1] = ofs + (i + 1) * width + (j + 0);
        res[2] = ofs + (i + 0) * width + (j + 1);

        // 3, 1, 2
        res[3] = ofs + (i + 0) * width + (j + 1);
        res[4] = ofs + (i + 1) * width + (j + 0);
        res[5] = ofs + (i + 1) * width + (j + 1);

        res += 6;
      }
    }
  }
}
