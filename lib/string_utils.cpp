#include "string_utils.hpp"
#include <stdint.h>

using namespace std;

namespace world
{
  string ToString(char const* const format, ...)
  {
#ifdef _WIN32
    va_list arg;
    va_start(arg, format);

    const int len = _vscprintf(format, arg) + 1;

    char* buf = (char*)_alloca(len);
    vsprintf_s(buf, len, format, arg);
    va_end(arg);

    return string(buf);
#else
    static char buf[4 * 1024];
    va_list arg;
    va_start(arg, format);
    vsprintf(buf, format, arg);
    string res(buf);
    va_end(arg);
    return res;
#endif
  }

#ifdef _WIN32
  string WideCharToUtf8(const WCHAR* str)
  {
    int len = (int)wcslen(str);
    char* buf = (char*)_alloca(len * 2 + 1);
    int res = WideCharToMultiByte(CP_UTF8, 0, str, len, buf, len * 2 + 1, NULL, NULL);
    if (res == 0)
      return false;

    buf[res] = '\0';
    return string(buf);
  }

  bool WideCharToUtf8(LPCOLESTR unicode, int len, string* str)
  {
    if (!unicode)
      return false;

    char* buf = (char*)_alloca(len * 2 + 1);

    int res = WideCharToMultiByte(CP_UTF8, 0, unicode, len, buf, len * 2 + 1, NULL, NULL);
    if (res == 0)
      return false;

    buf[res] = '\0';

    *str = string(buf);
    return true;
  }
#endif
  string Trim(const string& str)
  {
    int leading = 0, trailing = 0;
    while (isspace((uint8_t)str[leading]))
      leading++;
    const size_t end = str.size() - 1;
    while (isspace((uint8_t)str[end - trailing]))
      trailing++;

    return leading || trailing ? str.substr(leading, str.size() - (leading + trailing)) : str;
  }

  bool StartsWith(const char* str, const char* sub_str)
  {
    const size_t len_a = strlen(str);
    const size_t len_b = strlen(sub_str);
    if (len_a < len_b)
      return false;

    for (; *sub_str; ++str, ++sub_str)
    {
      if (*sub_str != *str)
        return false;
    }

    return true;
  }

  bool StartsWith(const string& str, const string& sub_str)
  {
    const size_t len_a = str.size();
    const size_t len_b = sub_str.size();
    if (len_a < len_b)
      return false;

    for (size_t i = 0; i < len_b; ++i)
    {
      if (sub_str[i] != str[i])
        return false;
    }

    return true;
  }
#ifdef _WIN32
  wstring Utf8ToWide(const char* str)
  {
    int len = (int)strlen(str);
    WCHAR* buf = (WCHAR*)_alloca((len + 1) * 2);

    wstring res;
    if (MultiByteToWideChar(CP_UTF8, 0, str, -1, buf, (len + 1) * 2))
    {
      res = wstring(buf);
    }
    else
    {
      int err = GetLastError();
    }
    return res;
  }
#endif

  //------------------------------------------------------------------------------
  vector<string> StringSplit(const string& str, char delim)
  {
    vector<string> res;
    const char* start = str.c_str();
    const char* end = str.c_str() + str.size();

    const char* cur = start;
    const char* prevStart = start;
    while (cur != end)
    {
      if (*cur == delim)
      {
        res.push_back(string(prevStart, cur - prevStart));
        prevStart = cur + 1;
      }
      cur++;
    }

    if (prevStart < end)
    {
      res.push_back(string(prevStart, cur - prevStart));
    }

    return res;
  }

  //------------------------------------------------------------------------------
  std::string StringJoin(const std::vector<std::string>& v, const std::string& delim)
  {
    string res;
    size_t cnt = v.size();

    for (size_t i = 0; i < cnt; ++i)
    {
      res += v[i];
      if (i != cnt - 1)
        res += delim;
    }

    return res;
  }
}
