#pragma once
#include <string>
#ifdef _WIN32
#include <atlconv.h>
#endif

namespace world
{
#ifdef _WIN32
  bool WideCharToUtf8(LPCOLESTR unicode, int len, std::string *str);
  std::string WideCharToUtf8(const WCHAR *str);
#endif

  std::string ToString(char const * const format, ... );
  std::string Trim(const std::string &str);

  std::string WideCharToUtf8(const std::wstring &str);

  std::wstring Utf8ToWide(const char *str);

  bool BeginsWith(const char *str, const char *sub_str);
  bool BeginsWith(const std::string &str, const std::string &sub_str);

  std::vector<std::string> StringSplit(const std::string& str, char delim);
  std::string StringJoin(const std::vector<std::string>& v, const std::string& delim);

}
