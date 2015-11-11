#include "tano_math.hpp"

namespace world
{
  vec3 vec3::Zero = {0, 0, 0};

  //------------------------------------------------------------------------------
  vec3 FromSpherical(float r, float phi, float theta)
  {
    float yProj = r * sinf(theta);
    return vec3(yProj * cosf(phi), r * cosf(theta), yProj * sinf(phi));
  }

  //------------------------------------------------------------------------------
  vec3 FromSpherical(const Spherical& s)
  {
    // phi is angle around x axis, ccw, starting at 0 at the x-axis
    // theta is angle around the z axis
    float yProj = s.r * sinf(s.theta);
    return vec3(yProj * cosf(s.phi), s.r * cosf(s.theta), yProj * sinf(s.phi));
  }

  //------------------------------------------------------------------------------
  Spherical ToSpherical(const vec3& v)
  {
    float r = Length(v);
    if (r == 0.f)
      return {0, 0, 0};

    return {r, atan2f(v.z, v.x), acosf(v.y / r)};
  }

  //------------------------------------------------------------------------------
#if 0
  vec3 PointOnHemisphere(const vec3& axis)
  {
    vec3 tmp(randf(-1.f, +1.f), randf(-1.f, +1.f), randf(-1.f, +1.f));
    tmp = Normalize(tmp);
    if (Dot(axis, tmp) < 0)
      tmp = -1.f * tmp;

    return tmp;
  }
  //------------------------------------------------------------------------------
  vec3 RandomVector(float scaleX, float scaleY, float scaleZ)
  {
    return vec3(scaleX * randf(-1.f, +1.f), scaleY * randf(-1.f, +1.f), scaleZ * randf(-1.f, +1.f));
  }
#endif

  //------------------------------------------------------------------------------
  vec3 CardinalSplineBlend(const vec3& p0, const vec3& p1, const vec3& p2, const vec3& p3, float s)
  {
    float a = 0.5f;

    vec3 t1 = a * (p2 - p0);
    vec3 t2 = a * (p3 - p1);

    // P = h1 * P1 + h2 * P2 + h3 * T1 + h4 * T2;
    float s2 = s * s;
    float s3 = s2 * s;
    float h1 = +2 * s3 - 3 * s2 + 1;
    float h2 = -2 * s3 + 3 * s2;
    float h3 = s3 - 2 * s2 + s;
    float h4 = s3 - s2;

    return h1 * p1 + h2 * p2 + h3 * t1 + h4 * t2;
  }

  //------------------------------------------------------------------------------
  void CardinalSpline::Create(const vec3* pts, int numPoints, float scale)
  {
    assert(scale != 0);
    _scale = scale;
    _controlPoints.resize(numPoints);
    copy(pts, pts + numPoints, _controlPoints.begin());
  }

  //------------------------------------------------------------------------------
  vec3 CardinalSpline::Interpolate(float t) const
  {
    int numPoints = (int)_controlPoints.size();
    t /= _scale;
    int i = (int)t;

    int m = numPoints - 1;
    vec3 p0 = _controlPoints[min(m, max(0, i - 1))];
    vec3 p1 = _controlPoints[min(m, max(0, i + 0))];
    vec3 p2 = _controlPoints[min(m, max(0, i + 1))];
    vec3 p3 = _controlPoints[min(m, max(0, i + 2))];

    float s = t - (float)i;
    return CardinalSplineBlend(p0, p1, p2, p3, s);
  }

  //------------------------------------------------------------------------------
  // Returns a vector perpendicular to u, using the method by Hughes-Moller
  vec3 Perp(vec3 u)
  {
    vec3 a = Abs(u);
    vec3 v;
    if (a.x <= a.y && a.x <= a.z)
      v = vec3(0, -u.z, u.y);
    else if (a.y <= a.x && a.y <= a.z)
      v = vec3(-u.z, 0, u.x);
    else
      v = vec3(-u.y, u.x, 0);

    return Normalize(v);
  }

  //------------------------------------------------------------------------------
  mat4x4 mat4x4::Identity()
  {
    return mat4x4{
      1, 0, 0, 0,
      0, 1, 0, 0,
      0, 0, 1, 0,
      0, 0, 0, 1
    };
  }

  //------------------------------------------------------------------------------
  // http://msdn.microsoft.com/en-us/library/windows/desktop/bb205342(v=vs.85).aspx
  mat4x4 MatrixLookAtLH(const vec3& vFrom, const vec3& vAt, const vec3& vWorldUp)
  {
    // Get the z basis vector, which points straight ahead. This is the
    // difference from the eyepoint to the lookat point.
    vec3 vView = Normalize(vAt - vFrom);

    // Get the dot product, and calculate the projection of the z basis
    // vector onto the up vector. The projection is the y basis vector.
    float fDotProduct = Dot(vWorldUp, vView);

    vec3 vUp = vWorldUp - fDotProduct * vView;

    // If this vector has near-zero length because the input specified a
    // bogus up vector, let's try a default up vector
    float fLength;

    if (1e-6f > (fLength = Length(vUp)))
    {
      vUp[0] = 0.0f - vView[1] * vView[0];
      vUp[1] = 1.0f - vView[1] * vView[1];
      vUp[2] = 0.0f - vView[1] * vView[2];

      // If we still have near-zero length, resort to a different axis.
      if (1e-6f > (fLength = Length(vUp)))
      {
        vUp[0] = 0.0f - vView[2] * vView[0];
        vUp[1] = 0.0f - vView[2] * vView[1];
        vUp[2] = 1.0f - vView[2] * vView[2];

        if (1e-6f > (fLength = Length(vUp)))
        {
          vUp[0] = 1.0f - vView[2] * vView[0];
          vUp[1] = 0.0f - vView[2] * vView[1];
          vUp[2] = 0.0f - vView[2] * vView[2];
          if (1e-6f > (fLength = Length(vUp)))
          {
            fLength = 0.001f; // just pick an arbitrary length; this really should never happen anyway.
          }
        }
      }
    }

    // Normalize the y basis vector
    vUp.Normalize();

    // The x basis vector is found simply with the cross product of the y
    // and z basis vectors
    vec3 vRight = Cross(vUp, vView);

    // Start building the matrix. The first three rows contains the basis
    // vectors used to rotate the view to point at the lookat point
    mat4x4 matrix;
    matrix[0 * 4 + 0] = vRight[0];
    matrix[0 * 4 + 1] = vUp[0];
    matrix[0 * 4 + 2] = vView[0];
    matrix[0 * 4 + 3] = 0;

    matrix[1 * 4 + 0] = vRight[1];
    matrix[1 * 4 + 1] = vUp[1];
    matrix[1 * 4 + 2] = vView[1];
    matrix[1 * 4 + 3] = 0;

    matrix[2 * 4 + 0] = vRight[2];
    matrix[2 * 4 + 1] = vUp[2];
    matrix[2 * 4 + 2] = vView[2];
    matrix[2 * 4 + 3] = 0;

    // Do the translation values (rotations are still about the eyepoint)
    matrix[3 * 4 + 0] = -Dot(vFrom, vRight);
    matrix[3 * 4 + 1] = -Dot(vFrom, vUp);
    matrix[3 * 4 + 2] = -Dot(vFrom, vView);
    matrix[3 * 4 + 3] = 1;
    return matrix;
  }

  //------------------------------------------------------------------------------
  //      http://msdn.microsoft.com/en-us/library/windows/desktop/bb204940(v=vs.85).aspx
  mat4x4 MatrixOrthoLH(float w, float h, float zn, float zf)
  {
    // from https://msdn.microsoft.com/en-us/library/windows/desktop/bb205346(v=vs.85).aspx
    float d = zf - zn;

    return mat4x4{
      2 / w, 0, 0, 0,
      0, 2 / h, 0, 0,
      0, 0, 1 / d, 0,
      0, 0, -zn / d, 1 };
  }

  //------------------------------------------------------------------------------
  // http://msdn.microsoft.com/en-us/library/windows/desktop/bb205350(v=vs.85).aspx
  mat4x4 PerspectiveFovLH(float fov, float aspectRatio, float zn, float zf)
  {
    mat4x4 matrix;
    float yScale = 1 / tanf(fov / 2);
    float xScale = yScale / aspectRatio;

    matrix[0] = xScale;
    matrix[1] = 0;
    matrix[2] = 0;
    matrix[3] = 0;

    matrix[4] = 0;
    matrix[5] = yScale;
    matrix[6] = 0;
    matrix[7] = 0;

    matrix[8] = 0;
    matrix[9] = 0;
    matrix[10] = zf / (zf - zn);
    matrix[11] = 1;

    matrix[12] = 0;
    matrix[13] = 0;
    matrix[14] = -zn * zf / (zf - zn);
    matrix[15] = 0;
    return matrix;
  }

  //------------------------------------------------------------------------------
  //inline float Dot(const DirectX::SimpleMath::Plane& plane, const vec3& pt)
  //{
  //  // Note, this is just the dot product between the plane's normal and pt
  //  return plane.x * pt.x + plane.y * pt.y + plane.z * pt.z;
  //}

  ////------------------------------------------------------------------------------
  //inline float DistanceToPoint(const DirectX::SimpleMath::Plane& plane, const vec3& pt)
  //{
  //  return plane.x * pt.x + plane.y * pt.y + plane.z * pt.z + plane.w;
  //}

  ////------------------------------------------------------------------------------
  //int ClipPolygonAgainstPlane(int vertexCount, const vec3* vertex, const Plane& plane, vec3* result)
  //{
  //  // from http://www.terathon.com/code/clipping.html
  //  enum Side
  //  {
  //    polygonInterior = 1,
  //    polygonBoundary = 0,
  //    polygonExterior = -1
  //  };

  //  Side location[16];

  //  const float boundaryEpsilon = 1.0e-3F;

  //  int positive = 0;
  //  int negative = 0;

  //  for (int a = 0; a < vertexCount; a++)
  //  {
  //    float d = DistanceToPoint(plane, vertex[a]);
  //    if (d > boundaryEpsilon)
  //    {
  //      location[a] = polygonInterior;
  //      positive++;
  //    }
  //    else
  //    {
  //      if (d < -boundaryEpsilon)
  //      {
  //        location[a] = polygonExterior;
  //        negative++;
  //      }
  //      else
  //      {
  //        location[a] = polygonBoundary;
  //      }
  //    }
  //  }

  //  if (negative == 0)
  //  {
  //    for (int a = 0; a < vertexCount; a++)
  //      result[a] = vertex[a];
  //    return (vertexCount);
  //  }
  //  else if (positive == 0)
  //  {
  //    return (0);
  //  }

  //  int count = 0;
  //  int previous = vertexCount - 1;
  //  for (int index = 0; index < vertexCount; index++)
  //  {
  //    int loc = location[index];
  //    if (loc == polygonExterior)
  //    {
  //      if (location[previous] == polygonInterior)
  //      {
  //        const vec3& v1 = vertex[previous];
  //        const vec3& v2 = vertex[index];

  //        vec3 dv = v2 - v1;
  //        float t = DistanceToPoint(plane, v2) / Dot(plane, dv);
  //        result[count++] = v2 - dv * t;
  //      }
  //    }
  //    else
  //    {
  //      const vec3& v1 = vertex[index];
  //      if ((loc == polygonInterior) && (location[previous] == polygonExterior))
  //      {
  //        const vec3& v2 = vertex[previous];
  //        vec3 dv = v2 - v1;

  //        float t = DistanceToPoint(plane, v2) / Dot(plane, dv);
  //        result[count++] = v2 - dv * t;
  //      }

  //      result[count++] = v1;
  //    }

  //    previous = index;
  //  }

  //  return (count);
  //}

}

