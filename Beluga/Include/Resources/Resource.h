#pragma once
#include <string.h>
#include <vector>
#include <unordered_map>
#include <map>
#include <fstream>
#include <memory>
#include <list>

#if defined(_WIN32)
#include <Windows.h>
#endif

#include "Parshing/tinyxml2.h"

namespace Bel
{
    /**********************************************************************************************
                                    Resource Loaders
    **********************************************************************************************/
    class ResourceCache;
    class ResourceHandle;

    class IResourceLoader
    {
    public:
        virtual ~IResourceLoader() = 0 {}
        virtual std::string GetPattern() = 0;
        virtual bool UseRawFile() = 0;
        virtual unsigned int GetLoadedResourceSize(char* pRawBuffer, unsigned int rawSize) = 0;
        virtual bool LoadResource(char* pRawBuffer, unsigned int rawSize, std::shared_ptr<ResourceHandle> pHandle) = 0;
        virtual bool AddNullZero() { return false; }
    };

    class DefaultResourceLoader : public IResourceLoader
    {
    public:
       // Inherited via IResourceLoader
        DefaultResourceLoader() {}
        virtual ~DefaultResourceLoader() override {};
        virtual std::string GetPattern() override;
        virtual bool UseRawFile() override;
        virtual unsigned int GetLoadedResourceSize(char* pRawBuffer, unsigned int rawSize) override;
        virtual bool LoadResource(char* pRawBuffer, unsigned int rawSize, std::shared_ptr<ResourceHandle> pHandle) override;
    };

    class Resource
    {
    private:
        std::string m_name;

    public:
        Resource(const std::string& name)
            : m_name(name)
        {
        }

        const std::string& GetName() const { return m_name; }
    };

    class IResourceExtraData
    {
    public:
        virtual ~IResourceExtraData() = 0 {}
        virtual std::string ToString() = 0;
    };

    class IResourceFile
    {
    protected:
        ResourceCache* m_pCache;

    public:
        virtual ~IResourceFile() = 0 {}
        virtual bool Open() = 0;
        virtual int GetNumResources() const = 0;
        virtual std::shared_ptr<ResourceHandle> LoadResource(const std::string& path) = 0;
        virtual size_t GetRawResourceSize(const Resource& resource) = 0;
        void SetResourceCache(ResourceCache* pCache) { m_pCache = pCache; }
    };
    
    class ZlibFile
    {
    private:
        struct ResourceInfo
        {
            uint32_t m_compressed;
            uint32_t m_size;
            uint32_t m_offset;
        };
        std::unordered_map<std::string, ResourceInfo> m_info;

        uint32_t m_currentOffset;
        std::vector<std::vector<char>> m_pendingData;
        std::fstream m_file;
        ResourceCache* m_pCache;

    public:
        ZlibFile()
            : m_currentOffset(0)
            , m_pCache(nullptr)
        {
        }

        void AddResource(std::string path, std::vector<char> data);
        std::shared_ptr<ResourceHandle> LoadResource(std::string path);
        void Save(const std::string& path);
        bool Load(const std::string& path);
        void SetCache(ResourceCache* pCache) { m_pCache = pCache; }
    };

#if defined(_WIN32) == false
    using DWORD = unsigned long;
    using WORD = unsigned short;
#endif

    class ZipFile
    {
    public:
        using ZipContentsMap = std::map<std::string, int>;

        struct ZipLocalHeader
        {
            enum { SIGNATURE = 0xbb63938b };
            DWORD m_signature;
            WORD  m_version;
            WORD  m_flag;
            WORD  m_compression;
            WORD  m_modTime;
            WORD  m_modData;
            DWORD m_crc32;
            DWORD m_cSize;
            DWORD m_ucSize;
            WORD  m_fNameLen;
            WORD  m_extraLen;
        };
        struct ZipDirHeader
        {
            enum { SIGNATURE = 0x59490b02 };
            DWORD   m_signature;
            WORD    m_disk;
            WORD    m_startDisk;
            WORD    m_dirEntries;
            WORD    m_totalDirEntries;
            DWORD   m_dirSize;
            DWORD   m_dirOffset;
            WORD    m_cmntLen;
        };
        struct ZipDirFileHeader
        {
            enum { SIGNATURE = 0x870e15e5 };
            DWORD   m_signature;
            WORD    m_verMade;
            WORD    m_verNeeded;
            WORD    m_flag;
            WORD    m_compression;
            WORD    m_modTime;
            WORD    m_modDate;
            DWORD   m_crc32;
            DWORD   m_compSize;
            DWORD   m_uncompSize;
            WORD    m_fNameLen;
            WORD    m_extraLen;
            WORD    m_cmntLen;
            WORD    m_diskStart;
            WORD    m_intAttr;
            WORD    m_extAttr;
            DWORD   m_hdrOffset;

            char* GetName()    { return reinterpret_cast<char*>(this + 1); }
            char* GetExtra()   { return GetName() + m_fNameLen; }
            char* GetComment() { return GetExtra() + m_extraLen; }
        };

    private:
        ZipContentsMap m_contentsMap;

        FILE* m_pFile;
        std::string m_dirData;
        int m_numEntries;

        std::vector<ZipDirFileHeader*> m_dirs;

    public:
        ZipFile();
        virtual ~ZipFile();

        bool Initialize(const std::string& fileName);
        void End();

        bool ReadFile(int i, void* pBuffer);
        std::string GetFileName(int index) const;
        int GetNumFiles() const { return m_numEntries; }
        bool ReadLargeFile(int i, void* pBuffer, void(*ProgressCallback)(int, bool&));
        int Find(const std::string& path) const;
        int GetFileLength(int i) const;
    };

    class ResourceZipFile : public IResourceFile
    {
    private:
        std::shared_ptr<ZipFile> m_pZipFile;
        std::string m_resFileName;

    public:
        ResourceZipFile(const std::string& resFileName);
        virtual ~ResourceZipFile();

        // Inherited via IResourceFile
        virtual bool Open();
        virtual size_t GetRawResourceSize(const Resource& resource) override;
        virtual int GetNumResources() const override;
        //virtual int GetRawResource(const Resource& resource, char* pBuffer) override;
        //virtual std::string GetResourceName(int num) const override;
    };

    class ResourceZlibFile : public IResourceFile
    {
    private:
        std::shared_ptr<ZlibFile> m_pXmlFile;
        std::string m_resFileName;
    public:
        ResourceZlibFile(const std::string& resFileName);
        virtual ~ResourceZlibFile();

        virtual bool Open() override;
        // Inherited via IResourceFile
        virtual int GetNumResources() const override;
        virtual std::shared_ptr<ResourceHandle> LoadResource(const std::string& path) override;
        virtual size_t GetRawResourceSize(const Resource& resource) override;
    };

    class ResourceHandle
    {
    private:
        friend class ResourceCache;
    
    protected:
        Resource m_resource;
        std::vector<char> m_data;
        std::shared_ptr<IResourceExtraData> m_extra;
        ResourceCache* m_pResCache;

    public:
        ResourceHandle(const Resource& resource, std::vector<char> data);
        ResourceHandle(const Resource& resource, std::vector<char> data, ResourceCache* pResCache);
        virtual ~ResourceHandle();

        size_t GetSize()            const    { return m_data.size(); }
        std::string GetName()       const    { return m_resource.GetName(); }
        std::vector<char>& GetData()         { return m_data; }

        std::shared_ptr<IResourceExtraData> GetExtra()           { return m_extra; }
        void SetExtra(std::shared_ptr<IResourceExtraData> extra) { m_extra = extra; }
    };


    constexpr unsigned int kCacheSize = 1024;
    class ResourceCache
    {
    public:
        using ResourceHandleList = std::list<std::shared_ptr<ResourceHandle>>;
        using ResourceHandleMap = std::map<std::string, std::shared_ptr<ResourceHandle>>;
        using ResourceLoaders = std::list<std::shared_ptr<IResourceLoader>>;

    protected:
        ResourceHandleList m_lru;
        ResourceHandleMap  m_resources;
        ResourceLoaders    m_resourceLoaders;

        IResourceFile* m_pFile;

        unsigned int m_cacheSize;
        unsigned int m_allocated;
    
    public:
        ResourceCache(const unsigned int sizeInMb, IResourceFile* pResFile);
        ~ResourceCache();

        bool Initialize();
        void RegisterLoader(std::shared_ptr<IResourceLoader> pLoader);
        std::shared_ptr<ResourceHandle> GetHandle(Resource* pResource);

        //std::vector<std::string> Match(const std::string pattern);

        void Flush(void);

        void MemoryHasBeenFreed(unsigned int size);

    protected:
        bool MakeRoom(unsigned int size);
        char* Allocate(unsigned int size);
        void Free(std::shared_ptr<ResourceHandle> pGonner);

        std::shared_ptr<ResourceHandle> Load(Resource* pResource);
        std::shared_ptr<ResourceHandle> Find(Resource* pResource);
        void Update(std::shared_ptr<ResourceHandle> handle);

        void FreeOneResource();
    };
}