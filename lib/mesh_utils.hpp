#pragma once
#include <lib/tano_math.hpp>

namespace world
{
  vector<u32> GenerateQuadIndices(int numQuads);
  vector<u32> GenerateCubeIndices(int numCubes);
  vector<u32> GenerateCubeIndicesFaceted(int numCubes);

  vector<u32> CreateCylinderIndices(int numRingSegments, int numHeightSegments, bool segmented = false);

  vec3* AddCube(vec3* buf, const vec3& pos, float scale);
  vec3* AddCubeWithNormal(vec3* buf, const vec3& pos, float scale, bool singleFace = false);

  void GeneratePlaneIndices(int width, int height, int ofs, vector<u32>* out);
}
