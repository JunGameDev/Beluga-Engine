#include <algorithm>
#include <optional>

#include "Resources/Resource.h"
#include "Core/Layers/ApplicationLayer.h"

#define ZLIB_WINAPI
#include "zlib.h"

using namespace Bel;
using namespace tinyxml2;

/******************************************************************************************
                                        XML files
******************************************************************************************/
void ZlibFile::AddResource(std::string path, std::vector<char> data)
{
    std::transform(path.begin(), path.end(), path.begin(), ::tolower);
    std::replace(path.begin(), path.end(), '\\', '/');

    ResourceInfo info;
    info.m_size = static_cast<uint32_t>(data.size());
    info.m_offset = m_currentOffset;


    std::vector<char> compressedData;
    compressedData.resize(data.size());

    z_stream stream;
    memset(&stream, 0, sizeof(stream));
    stream.avail_in = static_cast<uint32_t>(data.size());               // How much data are we gonna depress
    stream.next_in = reinterpret_cast<uint8_t*>(data.data());           // Where the pointer to that data ia
    stream.avail_out = static_cast<uint32_t>(compressedData.size());    // How much space we have available to write out into
    stream.next_out = reinterpret_cast<uint8_t*>(compressedData.data());// The actual pointer

    int result = deflateInit(&stream, Z_DEFAULT_COMPRESSION);
    if (result != Z_OK)
    {
        return;
    }

    result = deflate(&stream, Z_FINISH);

    if (result == Z_STREAM_END && stream.total_out < data.size() && stream.avail_in == 0)
    {
        //compressedData.resize(stream.total_out);
        compressedData = std::move(data);
    }
    else
    {
        // Compressed size is bigger, so just use uncompressed
        compressedData = std::move(data);
    }
    deflateEnd(&stream);

    info.m_compressed = static_cast<uint32_t>(compressedData.size());
    m_currentOffset += info.m_compressed;
    m_info[path] = info;
    m_pendingData.push_back(std::move(compressedData));
}

void ZlibFile::Save(const std::string& path)
{
    tinyxml2::XMLDocument doc;
    XMLElement* pRoot = doc.NewElement("ResourceFile");
    doc.InsertFirstChild(pRoot);

    for (auto& info : m_info)
    {
        XMLElement* pElement = doc.NewElement("Resource");
        pElement->SetAttribute("Path", info.first.c_str());
        pElement->SetAttribute("Compressed", info.second.m_compressed);
        pElement->SetAttribute("Size", info.second.m_size);
        pElement->SetAttribute("Offset", info.second.m_offset);
        pRoot->InsertEndChild(pElement);
    }

    XMLPrinter print;
    doc.Print(&print);

    m_file.open(path, std::ios_base::out | std::ios_base::binary);
    if (m_file.is_open())
    {
        for (auto& data : m_pendingData)
        {
            m_file.write(data.data(), data.size());
        }
        m_file.write(print.CStr(), print.CStrSize());

        // write out the size of our header
        int size = print.CStrSize();
        m_file.write(reinterpret_cast<char*>(&size), sizeof(int));

        m_pendingData.clear();
        m_info.clear();
        m_currentOffset = 0;
    }
    m_file.close();
}

bool ZlibFile::Load(const std::string& path)
{
    m_file.open(path, std::ios_base::in | std::ios_base::binary);
    if (m_file.is_open())
    {
        int sizeofInt = sizeof(int);
        m_file.seekg(-sizeofInt, m_file.end);
        int headerSize;
        m_file.read(reinterpret_cast<char*>(&headerSize), sizeof(int));
        m_file.seekg(-(headerSize + sizeofInt), m_file.cur);

        std::vector<char> header(headerSize);
        m_file.read(header.data(), headerSize);

        tinyxml2::XMLDocument doc;
        XMLError error = doc.Parse(header.data(), headerSize);
        if (error != XML_SUCCESS)
        {
            return false;
        }

        XMLElement* pRoot = doc.FirstChildElement();
        std::string name(pRoot->Name());
        if (name == "ResourceFile")
        {
            for (XMLElement* pElement = pRoot->FirstChildElement(); pElement;
                pElement = pElement->NextSiblingElement())
            {
                std::string elementName(pElement->Name());
                if (elementName == "Resource")
                {
                    ResourceInfo info;
                    info.m_compressed   = pElement->UnsignedAttribute("Compressed");
                    info.m_size         = pElement->UnsignedAttribute("Size");
                    info.m_offset       = pElement->UnsignedAttribute("Offset");
                    std::string path    = pElement->Attribute("Path");
                    if (!path.empty())
                    {
                        m_info[path] = info;
                    }
                }
            }
        }
        return true;
    }
    return false;
}

std::shared_ptr<ResourceHandle> ZlibFile::LoadResource(std::string path)
{
    if (!m_file.is_open())
    {
        return nullptr;
    }

    std::transform(path.begin(), path.end(), path.begin(), std::tolower);
    std::replace(path.begin(), path.end(), '\\', '/');

    auto itr = m_info.find(path);
    if (itr == m_info.end())
    {
        return nullptr;
    }

    std::vector<char> compressed(itr->second.m_compressed);
    m_file.seekg(itr->second.m_offset);
    m_file.read(compressed.data(), compressed.size());

    if (itr->second.m_size == itr->second.m_compressed)
    {
        return std::make_shared<ResourceHandle>(Resource(path), move(compressed), m_pCache);
    }

    std::vector<char> data(itr->second.m_size);

    z_stream stream;
    memset(&stream, 0, sizeof(stream));
    stream.avail_in     = static_cast<uint32_t>(compressed.size());
    stream.next_in      = reinterpret_cast<uint8_t*>(compressed.data());
    stream.avail_out    = static_cast<uint32_t>(data.size());
    stream.next_out     = reinterpret_cast<uint8_t*>(data.data());
    int result = inflateInit(&stream);
    if (result != Z_OK)
    {
        return nullptr;
    }

    result = inflate(&stream, Z_FINISH);
    if (result != Z_STREAM_END)
    {
        return nullptr;
    }

    return std::make_shared<ResourceHandle>(Resource(path), std::move(data), m_pCache);
}

/******************************************************************************************
                                        Zip files
******************************************************************************************/
ZipFile::ZipFile()
    : m_numEntries(0)
    , m_pFile(nullptr)
    , m_dirData(nullptr)
{

}

ZipFile::~ZipFile()
{
    End();
    fclose(m_pFile);
}

bool ZipFile::Initialize(const std::string& fileName)
{
    End();
    fopen_s(&m_pFile, fileName.c_str(), "rb");
    if (!m_pFile)
        return false;

    ZipDirHeader dirHeader;

    fseek(m_pFile, -static_cast<int>(sizeof(dirHeader)), SEEK_END);
    long headerOffset = ftell(m_pFile);
    memset(&dirHeader, 0, sizeof(dirHeader));

    if (dirHeader.m_signature != ZipDirHeader::SIGNATURE)
        return false;

    // Go to the beginning of the directory.
    fseek(m_pFile, headerOffset - dirHeader.m_dirSize, SEEK_SET);

    // Allocate the data buffer, and read the whole thing.
    m_dirData.resize(dirHeader.m_dirSize + dirHeader.m_dirEntries * sizeof(m_dirs), '\0');

    if (m_dirData.size() == 0)
        return false;

    fread(&m_dirData[0], dirHeader.m_dirSize, 1, m_pFile);

    char* pFileHeader = &m_dirData[0];

    //m_dirs = reinter

    bool success = true;

    for (int i = 0; i < dirHeader.m_dirEntries && success; ++i)
    {
        ZipDirFileHeader& fileHeader = *reinterpret_cast<ZipDirFileHeader*>(pFileHeader);

        m_dirs[i] = &fileHeader;

        // Check the directory entry integrity.
        if (fileHeader.m_signature != ZipDirFileHeader::SIGNATURE)
        {
            success = false;
        }
        else
        {
            pFileHeader += sizeof(fileHeader);

            // Convert slashes to backlashes.
            for (int j = 0; j < fileHeader.m_fNameLen; ++j)
            {
                if (pFileHeader[j] == '/')
                    pFileHeader[j] = '\\';
            }

            char fileName[_MAX_PATH]; // 260
            memcpy(fileName, pFileHeader, fileHeader.m_fNameLen);
            fileName[fileHeader.m_fNameLen] = 0;
            _strlwr_s(fileName, _MAX_PATH);
            std::string path = fileName;
            m_contentsMap[path] = i;

            // Skip etc.
            pFileHeader += fileHeader.m_fNameLen + fileHeader.m_extraLen + fileHeader.m_cmntLen;
        }
    }
    if (!success)
    {
        m_dirData.clear();
    }
    else
    {
        m_numEntries = dirHeader.m_totalDirEntries;
    }

    return success;
}

int ZipFile::Find(const std::string& path) const
{
    std::string lowerCase = path;
    std::transform(lowerCase.begin(), lowerCase.end(), lowerCase.begin(), std::tolower);
    auto it = m_contentsMap.find(lowerCase);
    if (it == m_contentsMap.end())
        return -1;

    return it->second;
}

int ZipFile::GetFileLength(int i) const
{
    if (i < 0 || i >= m_numEntries)
        return -1;
    else
        return m_dirs[i]->m_uncompSize;
}

void ZipFile::End()
{
    m_contentsMap.clear();
    m_dirs.clear();
    m_numEntries = 0;
}

std::string ZipFile::GetFileName(int i) const
{
    std::string fileName = "";
    if (i > 0 && i < m_numEntries)
    {
        char dest[_MAX_PATH];
        memcpy(dest, m_dirs[i]->GetName(), m_dirs[i]->m_fNameLen);
        dest[m_dirs[i]->m_fNameLen] = '\0';
        fileName = dest;
    }

    return fileName;
}

bool ZipFile::ReadFile(int i, void* pBuffer)
{
    /**************************************
         Uncompress a file
    ***************************************/

    if (pBuffer == nullptr || i < 0 || i >= m_numEntries)
    {
        return false;
    }

    fseek(m_pFile, m_dirs[i]->m_hdrOffset, SEEK_SET);
    ZipLocalHeader header;

    memset(&header, 0, sizeof(header));
    fread(&header, sizeof(header), 1, m_pFile);
    if (header.m_signature != ZipLocalHeader::SIGNATURE)
        return false;

    // Skip extra fields
    fseek(m_pFile, header.m_fNameLen + header.m_extraLen, SEEK_CUR);

    if (header.m_compression == Z_NO_COMPRESSION)
    {
        fread(pBuffer, header.m_cSize, 1, m_pFile);
        return true;
    }
    else if (header.m_compression != Z_DEFLATED)
    {
        return false;
    }

    // Alloc compressed data buffer and read the whole stream
    char* pCompData = new char[header.m_cSize];
    if (!pCompData)
        return false;

    memset(pCompData, 0, header.m_cSize);
    fread(pCompData, header.m_cSize, 1, m_pFile);

    bool toReturn = true;

    z_stream stream;
    int err;

    stream.avail_in = static_cast<uint32_t>(header.m_cSize);               // How much data are we gonna depress
    stream.next_in = reinterpret_cast<uint8_t*>(pCompData);
    stream.next_out = reinterpret_cast<uint8_t*>(pBuffer);
    stream.avail_out = static_cast<uint32_t>(header.m_ucSize);
    stream.zalloc = (alloc_func)0;
    stream.zfree = (free_func)0;

    // Inflation.
    err = inflateInit2(&stream, -MAX_WBITS);
    if (err == Z_OK)
    {
        err = inflate(&stream, Z_FINISH);
        inflateEnd(&stream);
        if (err == Z_STREAM_END)
            err = Z_OK;
        inflateEnd(&stream);
    }

    if (err != Z_OK)
        toReturn = false;

    delete[] pCompData;
    return toReturn;
}

bool ZipFile::ReadLargeFile(int i, void* pBuffer, void(*ProgressCallback)(int, bool&))
{
    if (pBuffer == nullptr || i < 0 || i >= m_numEntries)
    {
        return false;
    }

    fseek(m_pFile, m_dirs[i]->m_hdrOffset, SEEK_SET);
    ZipLocalHeader header;

    memset(&header, 0, sizeof(header));
    fread(&header, sizeof(header), 1, m_pFile);
    if (header.m_signature != ZipLocalHeader::SIGNATURE)
        return false;

    fseek(m_pFile, header.m_fNameLen + header.m_extraLen, SEEK_CUR);

    if (header.m_compression == Z_NO_COMPRESSION)
    {
        fread(pBuffer, header.m_cSize, 1, m_pFile);
        return true;
    }
    else if (header.m_compression != Z_DEFLATED)
        return false;

    char* pCompData = new char[header.m_compression];
    if (!pCompData)
        return false;

    memset(pCompData, 0, header.m_cSize);
    fread(pCompData, header.m_cSize, 1, m_pFile);

    bool toReturn = true;

    z_stream stream;
    int err;

    stream.next_in = reinterpret_cast<uint8_t*>(pCompData);
    stream.avail_in = static_cast<uint32_t>(header.m_cSize);
    stream.next_out = reinterpret_cast<uint8_t*>(pBuffer);
    stream.avail_out = (128 * 1024); //  read 128k at a time h.ucSize;
    stream.zalloc = (alloc_func)0;
    stream.zfree = (free_func)0;

    // Perform inflation
    // wbits < 0 => no zlib header inside the data.
    err = inflateInit2(&stream, -MAX_WBITS);
    if (err == Z_OK)
    {
        uInt count = 0;
        bool cancel = false;

        while (stream.total_in < static_cast<uInt>(header.m_cSize) && !cancel)
        {
            err = inflate(&stream, Z_SYNC_FLUSH);
            if (err == Z_STREAM_END)
            {
                err = Z_OK;
                break;
            }
            else if (err != Z_OK)
            {
                break;
            }

            stream.avail_out = (128 * 1024);
            stream.next_out += stream.total_out;

            ProgressCallback(count * 1000 / header.m_cSize, cancel);
        }
        inflateEnd(&stream);
    }

    if (err != Z_OK)
        toReturn = false;

    delete[] pCompData;
    return toReturn;
}

/******************************************************************************************
                                      Resource Handle
******************************************************************************************/
ResourceHandle::ResourceHandle(const Resource& resource, std::vector<char> data)
    : m_resource(resource)
    , m_data(data)
    , m_extra(nullptr)
    , m_pResCache(nullptr)
{
}

ResourceHandle::ResourceHandle(const Resource& resource, std::vector<char> data, ResourceCache* pResourceCache)
    : m_resource(resource)
    , m_data(data)
    , m_extra(nullptr)
    , m_pResCache(pResourceCache)
{
}

ResourceHandle::~ResourceHandle()
{
//    m_pResCache->MemoryHasBeenFreed(m_data.size());
//    m_data.clear();
}

/******************************************************************************************
                                      Resource Cache
******************************************************************************************/
ResourceCache::ResourceCache(const unsigned int sizeInMb, IResourceFile* pResFile)
    : m_cacheSize(sizeInMb * kCacheSize * kCacheSize)
    , m_pFile(pResFile)
    , m_allocated(0)
{
}

ResourceCache::~ResourceCache()
{
    while (!m_lru.empty())
    {
        FreeOneResource();
    }
}

bool ResourceCache::Initialize()
{
    bool ret = false;
    m_pFile->SetResourceCache(this);
    if (m_pFile->Open())
    {
        RegisterLoader(std::make_shared<DefaultResourceLoader>());
        ret = true;
    }

    return ret;
}

void ResourceCache::RegisterLoader(std::shared_ptr<IResourceLoader> pLoader)
{
    m_resourceLoaders.push_front(pLoader);
}

std::shared_ptr<ResourceHandle> ResourceCache::GetHandle(Resource* pResource)
{
    std::shared_ptr<ResourceHandle> pHandle(Find(pResource));
    if (pHandle == nullptr)
    {
        pHandle = Load(pResource);
    }
    else
    {
        Update(pHandle);
    }
    return pHandle;
}

bool WildcardMatch(const char* pStr, const char* pPattern)
{
    const size_t kLhsSize = strlen(pStr);
    const size_t kRhsSize = strlen(pPattern);

    if (kLhsSize == 0)
        return (kRhsSize == 0);

    std::vector<std::vector<bool>> lookUp;
    lookUp.resize(kLhsSize + 1);
    for (auto& temp : lookUp)
    {
        temp.resize(kRhsSize + 1);
    }

    lookUp[0][0] = true;

    for (size_t i = 1; i <= kRhsSize; ++i)
    {
        if (pPattern[i - 1] == '*')
        {
            lookUp[0][i] = lookUp[0][i - 1];
        }
    }

    for (size_t i = 1; i <= kLhsSize; ++i)
    {
        for (size_t j = 1; j <= kRhsSize; ++j)
        {
            if (pPattern[j - 1] == '*')
            {
                lookUp[i][j] = lookUp[i][j - 1] || lookUp[i - 1][j];
            }
            else if (pPattern[j - 1] == '?' || pStr[i - 1] == pPattern[j - 1])
            {
                lookUp[i][j] = lookUp[i - 1][j - 1];
            }
            else
            {
                lookUp[i][j] = false;
            }
        }
    }

    return lookUp[kLhsSize][kRhsSize];
}

std::shared_ptr<ResourceHandle> ResourceCache::Load(Resource* pResource)
{
    std::shared_ptr<IResourceLoader> pLoader;
    std::shared_ptr<ResourceHandle> pHandle;

    for (ResourceLoaders::iterator it = m_resourceLoaders.begin(); it != m_resourceLoaders.end(); ++it)
    {
        std::shared_ptr<IResourceLoader> pTemp = *it;

        if (WildcardMatch(pResource->GetName().c_str(), pTemp->GetPattern().c_str()))
        {
            pLoader = pTemp;
            break;
        }
    }

    if (!pLoader)
    {
        LOG_ERROR("Default resource loader not found!");
        return pHandle;
    }

    //unsigned int rawSize = m_pFile->(*pResource);
    pHandle = m_pFile->LoadResource(pResource->GetName());
    size_t rawSize = pHandle->GetSize();
    if (rawSize < 0)
    {
        LOG_ERROR("Resource size returned -1");
        return nullptr;
    }

    size_t allocSize = rawSize + ((pLoader->AddNullZero()) ? (1) : (0));
    char* pMem = Allocate(static_cast<unsigned int>(allocSize));
    
    if (!pMem)
    {
        LOG_ERROR("Resource cache out of memeory");
        return nullptr;
    }

    if (pHandle)
    {
        m_lru.push_front(pHandle);
        m_resources[pResource->GetName()] = pHandle;
    }

    return pHandle;
}

void ResourceCache::Flush(void)
{
    while (!m_lru.empty())
    {
        std::shared_ptr<ResourceHandle> pHandle = *(m_lru.begin());
        Free(pHandle);
        m_lru.pop_front();
    }
}

bool ResourceCache::MakeRoom(unsigned int size)
{
    if (size > m_cacheSize)
    {
        return false;
    }

    while (size > (m_cacheSize - m_allocated))
    {
        if (m_lru.empty())
            return false;

        FreeOneResource();
    }

    return true;
}

char* ResourceCache::Allocate(unsigned int size)
{
    if (!MakeRoom(size))
        return nullptr;

    char* pMem = new char[size];
    
    if (pMem)
    {
        m_allocated += size;
    }

    return pMem;
}

void ResourceCache::Free(std::shared_ptr<ResourceHandle> pGonner)
{
    m_lru.remove(pGonner);
    m_resources.erase(pGonner->m_resource.GetName());
}

std::shared_ptr<ResourceHandle> ResourceCache::Find(Resource* pResource)
{
    ResourceHandleMap::iterator iter = m_resources.find(pResource->GetName());
    if (iter == m_resources.end())
        return nullptr;

    return iter->second;
}

void ResourceCache::Update(std::shared_ptr<ResourceHandle> pHandle)
{ 
    m_lru.remove(pHandle);
    m_lru.push_front(pHandle);
}

void ResourceCache::FreeOneResource()
{
    ResourceHandleList::iterator gonner = m_lru.end();
    --gonner;

    std::shared_ptr<ResourceHandle> pHandle = *gonner;

    m_lru.pop_back();
    m_resources.erase(pHandle->m_resource.GetName());
    MemoryHasBeenFreed(static_cast<unsigned int>(pHandle->GetSize())); // Warning because of coversion from 'size_t' to 'unsigned int'???? What the hell???
    pHandle.reset();
}

void ResourceCache::MemoryHasBeenFreed(unsigned int size)
{
    m_allocated -= size;
}

//std::vector<std::string> ResourceCache::Match(const std::string pattern)
//{
//    std::vector<std::string> matchingNames;
//    if (m_pFile == nullptr)
//        return matchingNames;
//
//    int numFiles = m_pFile->GetNumResources();
//    for (int i = 0; i < numFiles; ++i)
//    {
//        std::string name = m_pFile->GetResourceName(i);
//        std::transform(name.begin(), name.end(), name.begin(), (int(*)(int)) std::tolower);
//        
//        if (WildcardMatch(pattern.c_str(), name.c_str()))
//        {
//            matchingNames.push_back(name);
//        }
//    }
//    return matchingNames;
//}

/******************************************************************************************
                                      Resource Zip File
******************************************************************************************/
ResourceZipFile::ResourceZipFile(const std::string& resFileName)
    : m_resFileName(resFileName)
{
}

ResourceZipFile::~ResourceZipFile()
{
}

bool ResourceZipFile::Open()
{
    m_pZipFile = std::make_shared<ZipFile>();

    if (m_pZipFile)
    {
        return m_pZipFile->Initialize(m_resFileName.c_str());
    }
    return false;
}

size_t ResourceZipFile::GetRawResourceSize(const Resource& resource)
{
    int resourceNum = m_pZipFile->Find(resource.GetName().c_str());
    if (resourceNum == -1)
        return -1;
    return m_pZipFile->GetFileLength(resourceNum);
}


int ResourceZipFile::GetNumResources() const
{
    return (m_pZipFile==nullptr) ? 0 : m_pZipFile->GetNumFiles();
}

//int ResourceZipFile::GetRawResource(const Resource& resource, char* pBuffer)
//{
//    int size = 0;
//    std::optional<int> resourceNum = m_pZipFile->Find(resource.GetName().c_str());
//    if (resourceNum.has_value())
//    {
//        size = m_pZipFile->GetFileLength(*resourceNum);
//        m_pZipFile->ReadFile(*resourceNum, pBuffer);
//    }
//
//    return size;
//}
//
//std::string ResourceZipFile::GetResourceName(int num) const
//{
//    std::string resName = "";
//    if (m_pZipFile != nullptr && num >= 0 && num < m_pZipFile->GetNumFiles())
//    {
//        resName = m_pZipFile->GetFileName(num);
//    }
//
//    return resName;
//}

/******************************************************************************************
                                      Resource XML File
******************************************************************************************/
ResourceZlibFile::ResourceZlibFile(const std::string& resFileName)
    : m_resFileName(resFileName)
    , m_pXmlFile(std::make_shared<ZlibFile>())
{
}

ResourceZlibFile::~ResourceZlibFile()
{
}

bool ResourceZlibFile::Open()
{
    if (m_pXmlFile)
    {
        m_pXmlFile->SetCache(m_pCache);
        return m_pXmlFile->Load(m_resFileName);
    }
    return false;
}

size_t ResourceZlibFile::GetRawResourceSize(const Resource& resource)
{
    auto pResource = m_pXmlFile->LoadResource(resource.GetName());
    if (!pResource)
        return -1;
    return pResource->GetData().size();
}

int ResourceZlibFile::GetNumResources() const
{
    return m_pXmlFile.use_count();
}

std::shared_ptr<ResourceHandle> Bel::ResourceZlibFile::LoadResource(const std::string& path)
{
    return m_pXmlFile->LoadResource(path);
}

//std::string ResourceXmlFile::GetResourceName(int num) const
//{
//    std::string resName = "";
//    if (m_pXmlFile != nullptr && num >= 0 && num < GetNumResources())
//    {
//        resName = m_pXmlFile->f->GetFileName(num);
//    }
//
//    return resName;
//}

/******************************************************************************************
                                     Default ResourceLoader
******************************************************************************************/
std::string DefaultResourceLoader::GetPattern()
{
    return "*";
}

bool DefaultResourceLoader::UseRawFile()
{
    return true;
}

unsigned int DefaultResourceLoader::GetLoadedResourceSize(char* pRawBuffer, unsigned int rawSize)
{
    return rawSize;
}

bool DefaultResourceLoader::LoadResource(char* pRawBuffer, unsigned int rawSize, std::shared_ptr<ResourceHandle> pHandle)
{
    return true;
}