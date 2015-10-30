#pragma once

#if !defined(NOMINMAX)
#define NOMINMAX
#endif

#define WITH_UNPACKED_RESOURCES 1
#define WITH_IMGUI 1

#include <sys/types.h>
#include <errno.h>
#include <fcntl.h>

#include <stdint.h>
#include <assert.h>
#include <time.h>
#include <direct.h>
#include <sys/stat.h>
#include <io.h>

#if WITH_IMGUI
#include <imgui/imgui.h>
#endif

#pragma warning(push)
#pragma warning(disable: 4005)
#include <dxgi.h>
#include <dxgidebug.h>
#include <d3d11.h>
#include <D3DX11tex.h>
//#include <DirectXMath.h>
#pragma warning(pop)

#include <atlbase.h>
#include <windows.h>
#include <windowsx.h>

#include <vector>
#include <set>
#include <unordered_set>
#include <map>
#include <unordered_map>
#include <string>
#include <queue>
#include <functional>
#include <memory>
#include <thread>

#include <stb/stb_image.h>

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int8_t s8;
typedef int16_t s16;
typedef int32_t s32;
typedef int64_t s64;


namespace world
{
  using std::vector;
  using std::string;
  using std::deque;
  using std::unordered_map;
  using std::unordered_set;
  using std::set;
  using std::map;
  using std::pair;
  using std::make_pair;
  using std::function;

  using std::conditional;
  using std::is_void;

  using std::min;
  using std::max;

  using std::unique_ptr;

  template<typename T>
  void hash_combine(size_t& seed, const T& key)
  {
    std::hash<T> hasher;
    seed ^= hasher(key) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
  }

  template<typename T1, typename T2>
  struct std::hash<std::pair<T1, T2>>
  {
    size_t operator()(const pair<T1, T2>& p) const
    {
      size_t seed = 0;
      hash_combine(seed, p.first);
      hash_combine(seed, p.second);
      return seed;
    }
  };
}

#pragma comment(lib, "DXGI.lib")
#pragma comment(lib, "DXGUID.lib")
#pragma comment(lib, "D3D11.lib")
#pragma comment(lib, "D3DX11.lib")
#pragma comment(lib, "psapi.lib")
#pragma comment(lib, "winmm.lib")
