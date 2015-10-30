#include "core/graphics.hpp"
#include "core/resource_manager.hpp"
#include "core/sprite_manager.hpp"
#include "lib/error.hpp"
#include "lib/init_sequence.hpp"
#include "lib/rolling_average.hpp"
#include "lib/stop_watch.hpp"
#include "lib/arena_allocator.hpp"
#include "lib/file_utils.hpp"
#include "world.hpp"
#include "game/level.hpp"

#if WITH_IMGUI
#include "core/imgui_helpers.hpp"
#endif

const TCHAR* g_AppWindowClass = _T("World");
const TCHAR* g_AppWindowTitle = _T("World");

using namespace world;
static const int WM_APP_CLOSE = WM_APP + 2;

ArenaAllocator world::g_ScratchMemory;
KeyUpTrigger world::g_KeyUpTrigger;

namespace world
{
  struct Entity
  {
    u32 id;
  };
}

//------------------------------------------------------------------------------
bool World::Init(HINSTANCE hinstance)
{
  BEGIN_INIT_SEQUENCE();

  FindAppRoot("app.gb");

  INIT_FATAL(ResourceManager::Create("resources.txt", _appRoot.c_str()));
  g_ResourceManager->AddPath("D:/OneDrive/world");
  g_ResourceManager->AddPath("C:/OneDrive/world");

  INIT_FATAL(Graphics::Create(hinstance));

  int width = GetSystemMetrics(SM_CXFULLSCREEN);
  int height = GetSystemMetrics(SM_CYFULLSCREEN);
  width = (int)(0.75f * width);
  height = (int)(0.75f * height);

  int bbWidth = width;
  int bbHeight = height;

  g_Graphics->CreateDefaultSwapChain(
    width, height, bbWidth, bbHeight, true, DXGI_FORMAT_R8G8B8A8_UNORM, WndProc, hinstance);

#if WITH_IMGUI
  INIT_FATAL(InitImGui(g_Graphics->GetSwapChain(g_Graphics->DefaultSwapChain())->_hwnd));
#endif

  INIT_FATAL(SpriteManager::Create());

  Level level;
  level.Load("gfx/level1.png");

  END_INIT_SEQUENCE();
}

//------------------------------------------------------------------------------
bool World::Close()
{
  SpriteManager::Destroy();
  ResourceManager::Destroy();
  Graphics::Destroy();
  return true;
}

//------------------------------------------------------------------------------
bool World::FindAppRoot(const char* filename)
{
  char starting_dir[MAX_PATH];
  _getcwd(starting_dir, MAX_PATH);

  // keep going up directory levels until we find @filename, or we hit the bottom..
  char prev_dir[MAX_PATH], cur_dir[MAX_PATH];
  ZeroMemory(prev_dir, sizeof(prev_dir));

  while (true)
  {
    _getcwd(cur_dir, MAX_PATH);
    // check if we haven't moved
    if (!strcmp(cur_dir, prev_dir))
      return false;

    memcpy(prev_dir, cur_dir, MAX_PATH);

    if (FileExists(filename))
    {
      _appRoot = cur_dir;
      return true;
    }

    if (_chdir("..") == -1)
      break;
  }
  _appRoot = starting_dir;
  return false;
}

//------------------------------------------------------------------------------
LRESULT World::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
#if WITH_IMGUI
  ImGuiWndProc(hWnd, message, wParam, lParam);
#endif

  switch (message)
  {
  case WM_APP_CLOSE: DestroyWindow(hWnd); break;

  case WM_DESTROY: PostQuitMessage(0); break;

//  case WM_LBUTTONUP: TANO._ioState.buttons[IoState::ButtonLeft] = false; break;
//
//  case WM_MBUTTONUP: TANO._ioState.buttons[IoState::ButtonMiddle] = false; break;
//
//  case WM_RBUTTONUP: TANO._ioState.buttons[IoState::ButtonRight] = false; break;
//
//  case WM_LBUTTONDOWN: TANO._ioState.buttons[IoState::ButtonLeft] = true; break;
//
//  case WM_MBUTTONDOWN: TANO._ioState.buttons[IoState::ButtonMiddle] = true; break;
//
//  case WM_RBUTTONDOWN: TANO._ioState.buttons[IoState::ButtonRight] = true; break;
//
//  case WM_MOUSEMOVE:
//    TANO._ioState.mouseX = (signed short)(lParam);
//    TANO._ioState.mouseY = (signed short)(lParam >> 16);
//    break;
//
//  case WM_MOUSEWHEEL:
//    TANO._ioState.mouseWheel += GET_WHEEL_DELTA_WPARAM(wParam) > 0 ? +1.0f : -1.0f;
//    break;
//
//  case WM_KEYDOWN:
//
//    switch (wParam)
//    {
//    case VK_LEFT:
//      g_DemoEngine->SetPos(g_DemoEngine->Pos() - TimeDuration::Seconds(1));
//      return 0;
//
//    case VK_RIGHT:
//      g_DemoEngine->SetPos(g_DemoEngine->Pos() + TimeDuration::Seconds(1));
//      return 0;
//
//    case VK_UP:
//      g_DemoEngine->SetPos(g_DemoEngine->Pos() - TimeDuration::Milliseconds(100));
//      return 0;
//
//    case VK_DOWN:
//      g_DemoEngine->SetPos(g_DemoEngine->Pos() + TimeDuration::Milliseconds(100));
//      return 0;
//    }
//    break;
//
  case WM_KEYUP:

    switch (wParam)
    {
    case VK_ESCAPE: PostQuitMessage(0); return 0;

    //case VK_SPACE: g_DemoEngine->SetPaused(!g_DemoEngine->Paused()); return 0;

    case VK_PRIOR:
      //g_DemoEngine->SetPos(g_DemoEngine->Pos() - TimeDuration::Seconds(30));
      return 0;

    case VK_NEXT:
      //g_DemoEngine->SetPos(g_DemoEngine->Pos() + TimeDuration::Seconds(30));
      return 0;

    //case VK_HOME: g_DemoEngine->SetPos(TimeDuration::Seconds(0)); return 0;

    default: break;
    }
    break;
  }
//
//  if (g_DemoEngine)
//  {
//    g_DemoEngine->WndProc(message, wParam, lParam);
//  }

  return DefWindowProc(hWnd, message, wParam, lParam);
}

//------------------------------------------------------------------------------
void World::UpdateIoState()
{
  u8 keystate[256];
  GetKeyboardState(keystate);
  for (int i = 0; i < 256; i++)
    _ioState.keysPressed[i] = (keystate[i] & 0x80) != 0;

  _ioState.controlPressed = (keystate[VK_CONTROL] & 0x80) != 0;
  _ioState.shiftPressed = (keystate[VK_SHIFT] & 0x80) != 0;
  _ioState.altPressed = (keystate[VK_MENU] & 0x80) != 0;
}

//------------------------------------------------------------------------------
bool World::Run()
{
  MSG msg = { 0 };

  RollingAverage<float> avgFrameTime(200);
  StopWatch stopWatch;
  u64 numFrames = 0;
  bool renderImgui = true;

  while (WM_QUIT != msg.message)
  {
    g_ScratchMemory.NewFrame();
    if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
    {
      TranslateMessage(&msg);
      DispatchMessage(&msg);
      continue;
    }
    stopWatch.Start();

    //UpdateIoState();

#if WITH_IMGUI
    UpdateImGui();
#endif

    g_Graphics->ClearRenderTarget(g_Graphics->GetBackBuffer());

#if WITH_UNPACKED_RESOUCES
    RESOURCE_MANAGER.Tick();
#endif

#if WITH_IMGUI
    float times[200];
    size_t numSamples;
    avgFrameTime.CopySamples(times, &numSamples);
    if (numSamples > 0)
    {
      ImGui::Begin("Perf", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
      float minValue, maxValue;
      avgFrameTime.GetMinMax(&minValue, &maxValue);
      ImGui::Text("Min: %.2f Max: %.2f Avg: %.2f",
        minValue * 1000,
        maxValue * 1000,
        avgFrameTime.GetAverage() * 1000);
      ImGui::PlotLines(
        "Frame time", times, (int)numSamples, 0, 0, FLT_MAX, FLT_MAX, ImVec2(200, 50));

      //// Invoke any custom perf callbacks
      //for (const fnPerfCallback& cb : _perfCallbacks)
      //  cb();
      ImGui::End();
    }

    if (g_KeyUpTrigger.IsTriggered('H'))
      renderImgui = !renderImgui;
    if (renderImgui)
      ImGui::Render();
#endif

    double frameTime = stopWatch.Stop();
    if (++numFrames > 10)
      avgFrameTime.AddSample((float)frameTime);

    g_Graphics->Present();
  }

  return true;
}

//------------------------------------------------------------------------------
int CALLBACK WinMain(HINSTANCE hinstance, HINSTANCE prev_instance, LPSTR cmd_line, int cmd_show)
{
  srand(1337);
#if WITH_MEMORY_TRACKING
  _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
  _CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_DEBUG);
  _CrtSetReportMode(_CRT_ERROR, _CRTDBG_MODE_DEBUG);
  _CrtSetReportMode(_CRT_ASSERT, _CRTDBG_MODE_DEBUG);
#endif

  World world;

  if (!world.Init(hinstance))
    return 1;

  world.Run();

  world.Close();

  return 0;
}
