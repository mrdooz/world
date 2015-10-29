#pragma once

#include <core/object_handle.hpp>
#include "filewatcher_win32.hpp"

namespace world
{
#if WITH_UNPACKED_RESOURCES

  class ResourceManager
  {
  public:
    ResourceManager(const char* outputFilename, const char* appRoot);
    ~ResourceManager();

    static bool Create(const char* outputFilename, const char* appRoot);
    static bool Destroy();

    bool FileExists(const char* filename);
    __time64_t ModifiedDate(const char* filename);
    bool LoadFile(const char* filename, vector<char>* buf);

    // file is opened relateive to the app root
    FILE* OpenWriteFile(const char* filename);
    template <typename T>
    void WriteFile(FILE* f, const T& t)
    {
      WriteFile(f, (const char*)&t, sizeof(t));
    }
    void WriteFile(FILE* f, const char* buf, int len);
    void CloseFile(FILE* f);

    ObjectHandle LoadTexture(const char* filename, bool srgb = false, D3DX11_IMAGE_INFO* info = nullptr);
    ObjectHandle LoadTextureFromMemory(const char* buf, u32 len, bool srgb, D3DX11_IMAGE_INFO* info);

    void AddPath(const string& path);

    FileWatcherWin32::AddFileWatchResult AddFileWatch(
        const string& filename, bool initial_callback, const FileWatcherWin32::cbFileChanged& cb);

    void RemoveFileWatch(FileWatcherWin32::WatchId id);

    void Tick();

  private:

    // Note, this is private because we don't want clients to actually know
    // the resolved path, but instead just use their friendly paths to identify
    // files (and call resman's LoadFile)
    string ResolveFilename(const char* filename, bool returnFullPath);

    FileWatcherWin32 _fileWatcher;

    vector<string> _paths;
    unordered_map<string, string> _resolvedPaths;

    string _outputFilename;

    struct FileInfo
    {
      FileInfo(const string& orgName, const string& resolvedName)
          : orgName(orgName), resolvedName(resolvedName)
      {
      }

      bool operator<(const FileInfo& rhs) const
      {
        return make_pair(orgName, resolvedName) < make_pair(rhs.orgName, rhs.resolvedName);
      }
      string orgName;
      string resolvedName;
    };

    set<FileInfo> _readFiles;
    string _appRoot;
  };

  extern ResourceManager* g_ResourceManager;

#else

  class PackedResourceManager
  {
  public:
    PackedResourceManager(const char* resourceFile);

    static bool Create(const char* resourceFile);
    static bool Destroy();

    bool LoadFile(const char* filename, vector<char>* buf);
    ObjectHandle LoadTexture(const char* filename,
        bool srgb = false,
        D3DX11_IMAGE_INFO* info = nullptr);

    ObjectHandle LoadTextureFromMemory(const char* buf, size_t len, bool srgb, D3DX11_IMAGE_INFO* info);

    FileWatcherWin32::AddFileWatchResult AddFileWatch(
      const string& filename, bool initial_callback, const FileWatcherWin32::cbFileChanged& cb);

  private:
    bool Init();
    int HashLookup(const char* key);

    struct PackedFileInfo
    {
      int offset;
      int compressedSize;
      int finalSize;
    };

    vector<char> _fileBuffer;
    vector<int> _intermediateHash;
    vector<int> _finalHash;
    vector<PackedFileInfo> _fileInfo;

    string _resourceFile;
  };

#endif
}
