#pragma once

namespace world
{
  enum VertexFlags
  {
    VF_POS            = 1 << 0,
    VF_POS_XY         = 1 << 1,
    VF_NORMAL         = 1 << 4,
    VF_TEX2_0         = 1 << 7,
    VF_TEX3_0         = 1 << 8,
    VF_COLOR          = 1 << 10,
    VF_COLOR_U32      = 1 << 11,

    // flags specifying order
    VF_ORDER_TEX_COL  = 1 << 16,
    VF_ORDER_CUSTOM   = 1 << 17,
  };

  //struct CD3D11_INPUT_ELEMENT_DESC : public D3D11_INPUT_ELEMENT_DESC
  //{
  //  CD3D11_INPUT_ELEMENT_DESC(LPCSTR name, DXGI_FORMAT format)
  //  {
  //    SemanticName = name;
  //    SemanticIndex = 0;
  //    Format = format;
  //    InputSlot = 0;
  //    AlignedByteOffset = 0;
  //    InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
  //    InstanceDataStepRate = 0;
  //  }
  //};

  uint32_t VertexSizeFromFlags(uint32_t flags);
  uint32_t IndexSizeFromFormat(DXGI_FORMAT format);
  uint32_t SizeFromFormat(DXGI_FORMAT format);

  uint32_t ColorToU32(float r, float g, float b, float a);
}