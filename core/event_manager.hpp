#pragma once

namespace world
{
  //------------------------------------------------------------------------------
  namespace event
  {
    enum EventType
    {
      kEventKeyDown,
      kEventKeyUp,

      kEventCount
    };

    struct EventBase
    {
      EventBase(EventType type) : type(type), len(0) {}
      EventType type;
      int len;
    };

    enum KeyModifiers
    {
      kModShift = 1 << 0,
      kModCtrl = 1 << 1,
      kModAlt = 1 << 2,
    };

    struct KeyDown : public EventBase
    {
      KeyDown(int key, u8 modifiers = 0) : EventBase(kEventKeyDown), key(key), modifiers(modifiers) {}
      int key;
      u8 modifiers = 0;
    };

    struct KeyUp : public EventBase
    {
      KeyUp(int key, u8 modifiers = 0) : EventBase(kEventKeyUp), key(key), modifiers(modifiers) {}
      int key;
      u8 modifiers = 0;
    };
  }

  //------------------------------------------------------------------------------
  struct EventManager
  {
    typedef function<void(const event::EventBase*)> fnEventListener;
    
    u32 RegisterListener(event::EventType type, const fnEventListener& listener);
    void UnregisterListener(u32 id);

    static bool Create();
    static bool Destroy();

    void Tick();

    template <typename T>
    void AddEvent(const T& event)
    {
      AddEvent(event.type, (void*)&event, sizeof(T));
    }

    void AddEvent(event::EventType type, void* data, int len);

    enum { EVENT_BUF_SIZE = 32 * 1024 * 1024 };
    char _eventBuf[EVENT_BUF_SIZE];
    u32 _eventOfs = 0;

    struct Listener
    {
      event::EventType type;
      fnEventListener listener;
    };

    vector<fnEventListener> _listeners[event::kEventCount];
  };

  extern EventManager* g_eventManager;
}