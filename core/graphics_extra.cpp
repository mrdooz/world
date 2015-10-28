#include "graphics_extra.hpp"
#include "graphics.hpp"
#include "graphics_context.hpp"
#include "graphics_utils.hpp"
#include <lib/arena_allocator.hpp>
#include <lib/init_sequence.hpp>
#include <lib/string_utils.hpp>
#include <_win32/resource.h>

using namespace world;

namespace world
{
  CD3D11_BLEND_DESC blendDescBlendSrcAlpha;
  CD3D11_BLEND_DESC blendDescBlendOneOne;
  CD3D11_BLEND_DESC blendDescPreMultipliedAlpha;
  CD3D11_BLEND_DESC blendDescWeightedBlend;
  CD3D11_BLEND_DESC blendDescWeightedBlendResolve;

  CD3D11_RASTERIZER_DESC rasterizeDescCullNone;
  CD3D11_RASTERIZER_DESC rasterizeDescCullFrontFace;
  CD3D11_RASTERIZER_DESC rasterizeDescWireframe;

  CD3D11_DEPTH_STENCIL_DESC depthDescDepthDisabled;
  CD3D11_DEPTH_STENCIL_DESC depthDescDepthWriteDisabled;

  //------------------------------------------------------------------------------
  void InitDefaultDescs()
  {
    blendDescBlendSrcAlpha = CD3D11_BLEND_DESC(CD3D11_DEFAULT());
    blendDescBlendSrcAlpha.IndependentBlendEnable = TRUE;
    blendDescBlendSrcAlpha.RenderTarget[0].BlendEnable = TRUE;
    blendDescBlendSrcAlpha.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
    blendDescBlendSrcAlpha.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;

    blendDescPreMultipliedAlpha = CD3D11_BLEND_DESC(CD3D11_DEFAULT());
    blendDescBlendSrcAlpha.IndependentBlendEnable = TRUE;
    blendDescPreMultipliedAlpha.RenderTarget[0].BlendEnable = TRUE;
    blendDescPreMultipliedAlpha.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
    blendDescPreMultipliedAlpha.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_INV_SRC_ALPHA;

    blendDescBlendOneOne = CD3D11_BLEND_DESC(CD3D11_DEFAULT());
    blendDescBlendSrcAlpha.IndependentBlendEnable = TRUE;
    blendDescBlendOneOne.RenderTarget[0].BlendEnable = TRUE;
    blendDescBlendOneOne.RenderTarget[0].SrcBlend = D3D11_BLEND_ONE;
    blendDescBlendOneOne.RenderTarget[0].DestBlend = D3D11_BLEND_ONE;

    blendDescWeightedBlend = CD3D11_BLEND_DESC(CD3D11_DEFAULT());
    blendDescWeightedBlend.IndependentBlendEnable = TRUE;
    blendDescWeightedBlend.RenderTarget[0].BlendEnable = TRUE;
    blendDescWeightedBlend.RenderTarget[0].SrcBlend = D3D11_BLEND_ONE;
    blendDescWeightedBlend.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
    blendDescWeightedBlend.RenderTarget[0].DestBlend = D3D11_BLEND_ONE;
    blendDescWeightedBlend.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ONE;
    blendDescWeightedBlend.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

    blendDescWeightedBlend.RenderTarget[1].BlendEnable = TRUE;
    blendDescWeightedBlend.RenderTarget[1].SrcBlend = D3D11_BLEND_ZERO;
    blendDescWeightedBlend.RenderTarget[1].SrcBlendAlpha = D3D11_BLEND_ZERO;
    blendDescWeightedBlend.RenderTarget[1].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
    blendDescWeightedBlend.RenderTarget[1].DestBlendAlpha = D3D11_BLEND_INV_SRC_ALPHA;
    blendDescWeightedBlend.RenderTarget[1].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

    blendDescWeightedBlendResolve = CD3D11_BLEND_DESC(CD3D11_DEFAULT());
    blendDescWeightedBlendResolve.IndependentBlendEnable = FALSE;
    blendDescWeightedBlendResolve.RenderTarget[0].BlendEnable = TRUE;
    blendDescWeightedBlendResolve.RenderTarget[0].SrcBlend = D3D11_BLEND_INV_SRC_ALPHA;
    blendDescWeightedBlendResolve.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_INV_SRC_ALPHA;
    blendDescWeightedBlendResolve.RenderTarget[0].DestBlend = D3D11_BLEND_SRC_ALPHA;
    blendDescWeightedBlendResolve.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_DEST_ALPHA;
    blendDescWeightedBlendResolve.RenderTarget[0].RenderTargetWriteMask =
        D3D11_COLOR_WRITE_ENABLE_ALL & ~D3D11_COLOR_WRITE_ENABLE_ALPHA;

    rasterizeDescCullNone = CD3D11_RASTERIZER_DESC(CD3D11_DEFAULT());
    rasterizeDescCullNone.CullMode = D3D11_CULL_NONE;

    rasterizeDescCullFrontFace = CD3D11_RASTERIZER_DESC(CD3D11_DEFAULT());
    rasterizeDescCullFrontFace.CullMode = D3D11_CULL_FRONT;

    rasterizeDescWireframe = CD3D11_RASTERIZER_DESC(CD3D11_DEFAULT());
    rasterizeDescWireframe.FillMode = D3D11_FILL_WIREFRAME;

    depthDescDepthDisabled = CD3D11_DEPTH_STENCIL_DESC(CD3D11_DEFAULT());
    depthDescDepthDisabled.DepthEnable = FALSE;

    depthDescDepthWriteDisabled = CD3D11_DEPTH_STENCIL_DESC(CD3D11_DEFAULT());
    depthDescDepthWriteDisabled.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
  }

  //------------------------------------------------------------------------------
  bool InitConfigDialog(HINSTANCE hInstance)
  {
    int res = (int)DialogBox(hInstance, MAKEINTRESOURCE(IDD_SETUP_DLG), NULL, DialogWndProc);
    return res == IDOK;
  }

  //------------------------------------------------------------------------------
  bool EnumerateDisplayModes(HWND hWnd)
  {
    deque<VideoAdapter>& videoAdapters = g_Graphics->_graphicsSettings.videoAdapters;
    videoAdapters.clear();

    // Create DXGI factory to enumerate adapters
    if (FAILED(CreateDXGIFactory1(IID_PPV_ARGS(&g_Graphics->_graphicsSettings.dxgi_factory))))
    {
      return false;
    }

    // Enumerate the adapters
    IDXGIAdapter* adapter;
    for (int i = 0; SUCCEEDED(g_Graphics->_graphicsSettings.dxgi_factory->EnumAdapters(i, &adapter)); ++i)
    {
      videoAdapters.push_back(VideoAdapter());
      VideoAdapter& curAdapter = videoAdapters.back();
      curAdapter.adapter = adapter;
      adapter->GetDesc(&curAdapter.desc);
      HWND hAdapter = GetDlgItem(hWnd, IDC_VIDEO_ADAPTER);
      ComboBox_AddString(hAdapter, WideCharToUtf8(curAdapter.desc.Description).c_str());
      ComboBox_SetCurSel(hAdapter, 0);
      g_Graphics->_graphicsSettings.selectedAdapter = 0;

      CComPtr<IDXGIOutput> output;
      vector<CComPtr<IDXGIOutput>> outputs;
      vector<DXGI_MODE_DESC> displayModes;

      // Enumerate the outputs on the adapter
      // Note(magnus): This is a bit dodgey, but EnumOutputs always fails for the second card on a
      // laptop with an integrated card (say an Intel).
      for (int j = 0; SUCCEEDED(adapter->EnumOutputs(j, &output)); ++j)
      {
        DXGI_OUTPUT_DESC outputDesc;
        output->GetDesc(&outputDesc);
        UINT numModes;
        output->GetDisplayModeList(DXGI_FORMAT_R8G8B8A8_UNORM, 0, &numModes, NULL);
        size_t prevSize = displayModes.size();
        displayModes.resize(displayModes.size() + numModes);
        output->GetDisplayModeList(
            DXGI_FORMAT_R8G8B8A8_UNORM, 0, &numModes, displayModes.data() + prevSize);
        if (!g_Graphics->_enumerateAllOutputs)
          break;
      }

      // Only keep the version of each display mode with the highest refresh rate
      auto& safeRational = [](const DXGI_RATIONAL& r)
      {
        return r.Denominator == 0 ? 0 : r.Numerator / r.Denominator;
      };
      if (g_Graphics->DisplayAllModes())
      {
        curAdapter.displayModes = displayModes;
      }
      else
      {
        unordered_map<pair<int, int>, DXGI_RATIONAL> highestRate;
        for (size_t i = 0; i < displayModes.size(); ++i)
        {
          auto& cur = displayModes[i];
          auto key = make_pair(cur.Width, cur.Height);
          if (safeRational(cur.RefreshRate) > safeRational(highestRate[key]))
          {
            highestRate[key] = cur.RefreshRate;
          }
        }

        for (auto it = begin(highestRate); it != end(highestRate); ++it)
        {
          DXGI_MODE_DESC desc;
          desc.Width = it->first.first;
          desc.Height = it->first.second;
          desc.RefreshRate = it->second;
          curAdapter.displayModes.push_back(desc);
        }

        auto& resSorter = [&](const DXGI_MODE_DESC& a, const DXGI_MODE_DESC& b)
        {
          return a.Width < b.Width;
        };

        sort(begin(curAdapter.displayModes), end(curAdapter.displayModes), resSorter);
      }

      HWND hDisplayMode = GetDlgItem(hWnd, IDC_DISPLAY_MODES);
      for (size_t k = 0; k < curAdapter.displayModes.size(); ++k)
      {
        auto& cur = curAdapter.displayModes[k];
        char buf[256];
        sprintf_s(buf, sizeof(buf), "%dx%d (%dHz)", cur.Width, cur.Height, safeRational(cur.RefreshRate));
        ComboBox_InsertString(hDisplayMode, k, buf);
        // encode width << 16 | height in the item data
        ComboBox_SetItemData(hDisplayMode, k, (cur.Width << 16) | cur.Height);
      }

      int cnt = ComboBox_GetCount(hDisplayMode);
      ComboBox_SetCurSel(hDisplayMode, cnt - 1);

      adapter->Release();
    }

    // If the first display adapter is "Microsoft Basic Render Driver", put in on the back of the list
    if (wcscmp(videoAdapters.front().desc.Description, L"Microsoft Basic Render Driver") == 0)
    {
      VideoAdapter tmp = videoAdapters.front();
      videoAdapters.pop_front();
      videoAdapters.push_back(tmp);
    }

    Button_SetCheck(GetDlgItem(hWnd, IDC_WINDOWED), FALSE);
    Button_SetCheck(GetDlgItem(hWnd, IDC_VSYNC), TRUE);
    return true;
  }

  //------------------------------------------------------------------------------
  INT_PTR CALLBACK DialogWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
  {
    switch (message)
    {
      case WM_INITDIALOG:
      {
        if (!EnumerateDisplayModes(hWnd))
        {
          EndDialog(hWnd, -1);
        }
#if !WITH_CONFIG_DLG
        if (!g_Graphics->_graphicsSettings.videoAdapters.empty())
        {
          ComboBox_SetCurSel(GetDlgItem(hWnd, IDC_DISPLAY_MODES),
              3 * g_Graphics->_graphicsSettings.videoAdapters[0].displayModes.size() / 4);
        }
        EndDialog(hWnd, 1);
#endif
        RECT rect;
        GetClientRect(hWnd, &rect);
        int width = GetSystemMetrics(SM_CXSCREEN);
        int height = GetSystemMetrics(SM_CYSCREEN);
        SetWindowPos(hWnd,
            NULL,
            width / 2 - (rect.right - rect.left) / 2,
            height / 2 - (rect.bottom - rect.top) / 2,
            -1,
            -1,
            SWP_NOZORDER | SWP_NOSIZE);
        break;
      }

      case WM_COMMAND:
        switch (LOWORD(wParam))
        {
          case IDCANCEL: EndDialog(hWnd, 0); return TRUE;

          case IDOK: EndDialog(hWnd, 1); return TRUE;
        }
        break; // end WM_COMMAND

      case WM_DESTROY:
      {
        // copy the settings over
        GraphicsSettings& cur = g_Graphics->_graphicsSettings;
        cur.windowed = !!Button_GetCheck(GetDlgItem(hWnd, IDC_WINDOWED));
        cur.vsync = !!Button_GetCheck(GetDlgItem(hWnd, IDC_WINDOWED));

        cur.SelectedDisplayMode = ComboBox_GetCurSel(GetDlgItem(hWnd, IDC_DISPLAY_MODES));

        HWND hDisplayModes = GetDlgItem(hWnd, IDC_DISPLAY_MODES);
        int sel = ComboBox_GetCurSel(hDisplayModes);
        int data = (int)ComboBox_GetItemData(hDisplayModes, sel);
        cur.width = HIWORD(data);
        cur.height = LOWORD(data);
        break;
      }
    }
    return FALSE;
  }

  //------------------------------------------------------------------------------
  void SetClientSize(HWND hwnd, int width, int height)
  {
    RECT client_rect;
    RECT window_rect;
    GetClientRect(hwnd, &client_rect);
    GetWindowRect(hwnd, &window_rect);
    window_rect.right -= window_rect.left;
    window_rect.bottom -= window_rect.top;
    const int dx = window_rect.right - client_rect.right;
    const int dy = window_rect.bottom - client_rect.bottom;

    SetWindowPos(hwnd, NULL, -1, -1, width + dx, height + dy, SWP_NOZORDER | SWP_NOREPOSITION);
  }

  //------------------------------------------------------------------------------
  void VertexFlagsToLayoutDesc(u32 vertexFlags, vector<D3D11_INPUT_ELEMENT_DESC>* desc)
  {
    u32 ofs = 0;
    if (vertexFlags & VF_POS)
    {
      desc->push_back(
          {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0});
      ofs += 12;
    }

    if (vertexFlags & VF_POS_XY)
    {
      desc->push_back(
          {"POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0});
      ofs += 8;
    }

    if (vertexFlags & VF_NORMAL)
    {
      desc->push_back(
          {"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, ofs, D3D11_INPUT_PER_VERTEX_DATA, 0});
      ofs += 12;
    }

    // check the ordering flag, to see if uvs or col should be added first
    if (vertexFlags & VF_ORDER_TEX_COL)
    {
      // uv before col
      if (vertexFlags & VF_TEX2_0)
      {
        desc->push_back(
            {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, ofs, D3D11_INPUT_PER_VERTEX_DATA, 0});
        ofs += 8;
      }

      if (vertexFlags & VF_TEX3_0)
      {
        desc->push_back(
            {"TEXCOORD", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, ofs, D3D11_INPUT_PER_VERTEX_DATA, 0});
        ofs += 12;
      }

      if (vertexFlags & VF_COLOR)
      {
        desc->push_back(
            {"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, ofs, D3D11_INPUT_PER_VERTEX_DATA, 0});
        ofs += 16;
      }

      if (vertexFlags & VF_COLOR_U32)
      {
        desc->push_back(
            {"COLOR", 0, DXGI_FORMAT_R8G8B8A8_UNORM, 0, ofs, D3D11_INPUT_PER_VERTEX_DATA, 0});
        ofs += 4;
      }
    }
    else
    {
      // col before uv
      if (vertexFlags & VF_COLOR)
      {
        desc->push_back(
            {"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, ofs, D3D11_INPUT_PER_VERTEX_DATA, 0});
        ofs += 16;
      }

      if (vertexFlags & VF_COLOR_U32)
      {
        desc->push_back(
            {"COLOR", 0, DXGI_FORMAT_R8G8B8A8_UNORM, 0, ofs, D3D11_INPUT_PER_VERTEX_DATA, 0});
        ofs += 4;
      }

      if (vertexFlags & VF_TEX2_0)
      {
        desc->push_back(
            {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, ofs, D3D11_INPUT_PER_VERTEX_DATA, 0});
        ofs += 8;
      }

      if (vertexFlags & VF_TEX3_0)
      {
        desc->push_back(
            {"TEXCOORD", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, ofs, D3D11_INPUT_PER_VERTEX_DATA, 0});
        ofs += 12;
      }
    }
  }
}

//------------------------------------------------------------------------------
bool SwapChain::CreateBackBuffers(u32 width, u32 height, DXGI_FORMAT format)
{
  BEGIN_INIT_SEQUENCE();

  _width = width;
  _height = height;

  RenderTargetResource* rt = new RenderTargetResource();

  INIT_HR_FATAL(_swapChain->GetBuffer(0, IID_PPV_ARGS(&rt->texture.ptr)));
  rt->texture.ptr->GetDesc(&rt->texture.desc);

  // Create render target view
  D3D11_RENDER_TARGET_VIEW_DESC rtViewDesc;
  ZeroMemory(&rtViewDesc, sizeof(rtViewDesc));
  rtViewDesc.Format = rt->texture.desc.Format;
  rtViewDesc.ViewDimension = rt->texture.desc.SampleDesc.Count == 1
                                 ? D3D11_RTV_DIMENSION_TEXTURE2D
                                 : D3D11_RTV_DIMENSION_TEXTURE2DMS;
  INIT_HR_FATAL(
      g_Graphics->_device->CreateRenderTargetView(rt->texture.ptr, &rtViewDesc, &rt->view.ptr));
  rt->view.ptr->GetDesc(&rt->view.desc);

  DepthStencilResource* depthStencil = new DepthStencilResource();

  CD3D11_TEXTURE2D_DESC depthStencilDesc(DXGI_FORMAT_D24_UNORM_S8_UINT,
      width,
      height,
      1,
      1,
      D3D11_BIND_DEPTH_STENCIL,
      D3D11_USAGE_DEFAULT,
      0,
      _desc.SampleDesc.Count);

  // Create depth stencil buffer and view
  INIT_HR_FATAL(
      g_Graphics->_device->CreateTexture2D(&depthStencilDesc, NULL, &depthStencil->texture.ptr));
  depthStencil->texture.ptr->GetDesc(&depthStencil->texture.desc);

  INIT_HR_FATAL(g_Graphics->_device->CreateDepthStencilView(
      depthStencil->texture.ptr, NULL, &depthStencil->view.ptr));
  depthStencil->view.ptr->GetDesc(&depthStencil->view.desc);

  // register the render-target and depth-stencil
  u32 rtIdx = g_Graphics->_renderTargets.Append(rt);
  _renderTarget = ObjectHandle(ObjectHandle::kRenderTarget, rtIdx);

  u32 dsIdx = g_Graphics->_depthStencils.Append(depthStencil);
  _depthStencil = ObjectHandle(ObjectHandle::kDepthStencil, rtIdx);

  _viewport = CD3D11_VIEWPORT(0.0f, 0.0f, (float)width, (float)height);

  END_INIT_SEQUENCE();
}

//------------------------------------------------------------------------------
void SwapChain::Present()
{
  _swapChain->Present(g_Graphics->_vsync ? 1 : 0, 0);
}

//------------------------------------------------------------------------------
ScopedRenderTarget::ScopedRenderTarget(
    int width, int height, DXGI_FORMAT format, const BufferFlags& bufferFlags)
    : _desc(width, height, format)
    , _rtHandle(g_Graphics->GetTempRenderTarget(width, height, format, bufferFlags))
{
}

//------------------------------------------------------------------------------
ScopedRenderTarget::ScopedRenderTarget(const RenderTargetDesc& desc, const BufferFlags& bufferFlags)
    : _desc(desc)
    , _rtHandle(g_Graphics->GetTempRenderTarget(desc.width, desc.height, desc.format, bufferFlags))
{
}

//------------------------------------------------------------------------------
ScopedRenderTarget::ScopedRenderTarget(DXGI_FORMAT format, const BufferFlags& bufferFlags)
{
  _desc.format = format;
  g_Graphics->GetBackBufferSize(&_desc.width, &_desc.height);
  _rtHandle = g_Graphics->GetTempRenderTarget(_desc.width, _desc.height, _desc.format, bufferFlags);
}

//------------------------------------------------------------------------------
ScopedRenderTarget::~ScopedRenderTarget()
{
  g_Graphics->ReleaseTempRenderTarget(_rtHandle);
}

//----------------------------------------------outputDe--------------------------------
ScopedRenderTargetFull::ScopedRenderTargetFull(
    DXGI_FORMAT format, BufferFlags rtFlags, BufferFlags dsFlags)
{
  _desc.format = format;
  g_Graphics->GetBackBufferSize(&_desc.width, &_desc.height);
  _rtHandle = g_Graphics->GetTempRenderTarget(_desc.width, _desc.height, format, rtFlags);
  _dsHandle = g_Graphics->GetTempDepthStencil(_desc.width, _desc.height, dsFlags);
}

//------------------------------------------------------------------------------
ScopedRenderTargetFull::~ScopedRenderTargetFull()
{
  g_Graphics->ReleaseTempRenderTarget(_rtHandle);
  g_Graphics->ReleaseTempDepthStencil(_dsHandle);
}
