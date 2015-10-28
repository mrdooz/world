#pragma once
#include <stdint.h>
#include <vector>
#include <sstream>
//#include "utils.hpp"

namespace world
{
  //----------------------------------------------------------------------------------
  enum LogLevel
  {
    LogLevelNone,
    LogLevelDebug,
    LogLevelInfo,
    LogLevelWarning,
    LogLevelError,
  };

  enum LogFlags
  {
    LogFlagsNaked   = 1 << 0,
  };

  struct LogEntry;
  typedef std::function<void (const LogEntry& entry)> fnLogCallback;
  void SetLogCallback(const fnLogCallback& cb);
  void SetBreakOnError(bool value);

  struct LogEntry
  {
    LogLevel level;
    uint32_t flags;
    const char* file;
    uint32_t line;
    const char* msg;
  };

  //----------------------------------------------------------------------------------
  struct LogSink
  {
    LogSink();
    virtual ~LogSink();
    virtual void Log(const LogEntry& entry) = 0;
  };

  //----------------------------------------------------------------------------------
  struct LogSinkFile : public LogSink
  {
    LogSinkFile();
    ~LogSinkFile();

    bool Open(const char* filename);
    virtual void Log(const LogEntry& entry) override;

    FILE* _file;
  };

  //----------------------------------------------------------------------------------
  struct LogSinkConsole : public LogSink
  {
    virtual void Log(const LogEntry& entry) override;
  };

  //----------------------------------------------------------------------------------
  struct LogSinkApp : public LogSink
  {
    virtual void Log(const LogEntry& entry) override;
  };

  //----------------------------------------------------------------------------------
  struct LogStream
  {
    LogStream(LogLevel level, const char* file, uint32_t line, uint32_t flags);
    ~LogStream();

    template <typename T>
    void Log(const T& t)
    {
      std::ostringstream str;
      str << t;
      _curMessage.append(str.str());
    }

    template <typename T, typename... Args>
    void Log(const T& head, Args... tail)
    {
      Log(head);
      Log(tail...);
    }

    std::string _curMessage;
    LogLevel _level;
    const char* _file;
    uint32_t _line;
    uint32_t _flags;
  };

  // The minimum level at which we log
  LogLevel GetLogLevel();
  void SetLogLevel(LogLevel level);

#define LOG_DEBUG(...)  \
  world::LogStream (world::LogLevelDebug, __FILE__, __LINE__, 0).Log(__VA_ARGS__);
#define LOG_INFO(...) \
  world::LogStream(world::LogLevelInfo, __FILE__, __LINE__, 0).Log(__VA_ARGS__);
#define LOG_WARN(...) \
  world::LogStream(world::LogLevelWarning, __FILE__, __LINE__, 0).Log(__VA_ARGS__);
#define LOG_ERROR(...)  \
  world::LogStream(world::LogLevelError, __FILE__, __LINE__, 0).Log(__VA_ARGS__);

#define LOG_INFO_NAKED(...) \
  world::LogStream(world::LogLevelInfo, __FILE__, __LINE__, LogFlagsNaked).Log(__VA_ARGS__);
#define LOG_WARN_NAKED(...) \
  world::LogStream(world::LogLevelWarning, __FILE__, __LINE__, LogFlagsNaked).Log(__VA_ARGS__);
#define LOG_ERROR_NAKED(...)  \
  world::LogStream(world::LogLevelError, __FILE__, __LINE__, LogFlagsNaked).Log(__VA_ARGS__);
}
