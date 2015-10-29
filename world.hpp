#pragma once

namespace world
{
  struct IoState
  {
    enum Button
    {
      ButtonLeft,
      ButtonMiddle,
      ButtonRight,
      NumButtons
    };

    float deltaTime;
    float mouseX, mouseY;
    float mouseWheel;
    bool buttons[NumButtons];
    u8 keysPressed[256];
    u8 shiftPressed : 1;
    u8 controlPressed : 1;
    u8 altPressed : 1;
  };

  struct KeyUpTrigger
  {
    KeyUpTrigger()
    {
      memset(triggers, 0, 512);
    }

    bool IsTriggered(int key)
    {
      bool tmp = triggers[key];
      triggers[key] = false;
      return tmp;
    }

    void SetTrigger(int key)
    {
      triggers[key] = true;
    }

    bool triggers[512];
  };

  extern KeyUpTrigger g_KeyUpTrigger;

  struct World
  {
    bool FindAppRoot(const char* filename);
    bool Init(HINSTANCE hinstance);
    bool Close();
    bool Run();

    static LRESULT WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

    void UpdateIoState();

    IoState _ioState;
    string _appRoot;
  };
}