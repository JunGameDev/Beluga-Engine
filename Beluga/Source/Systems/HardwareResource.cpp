// Copyright (c) 2020 Junyoung Kim

#include <string>
#include <sstream>

#if defined(_WIN32)
#include <Windows.h>
#include <VersionHelpers.h>
#endif

#include "Core/Layers/ApplicationLayer.h"
#include "Systems/HardwareResource.h"

using namespace Bel;

extern std::string WStrToStr(const std::wstring& kWstr)
{
	size_t length = static_cast<int>(kWstr.length() + 1);
	std::string ret(length, '\0');
	WideCharToMultiByte(CP_ACP, 0, kWstr.c_str(), length, &ret[0], length, 0, 0);
	return ret;
}

// Converting string ot wstring
// Solution from https://stackoverflow.com/questions/4804298/how-to-convert-wstring-into-string/18374698
extern std::wstring StrToWStr(const std::string& kStr)
{
	if (kStr.empty())
	{
		return std::wstring();
	}
	size_t len = kStr.length() + 1;
	std::wstring ret = std::wstring(len, 0);
#ifdef WIN32
	int size = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, &kStr[0], kStr.size(), &ret[0], len);
	ret.resize(size);
#else
#endif
	return ret;
}

#ifdef _WIN32
/********************************************
*											*
*	Classes to read hardware information    *
*   from local registry.                    *
*											*
*********************************************/
class WindowsProcessorInfoReader : public IProcessorInfoReader
{
private:
	Processor m_processor;

public:
	WindowsProcessorInfoReader()
	{
	}

	virtual ~WindowsProcessorInfoReader() override
	{
	}

	virtual bool CheckProcessorStatus()
	{
		SYSTEM_INFO systemInfo;
		GetSystemInfo(&systemInfo);

		return ReadProcessorName() | ReadProcessorType(systemInfo) | ReadProcessorFrequency(systemInfo) | ReadProcessorCache();
	}

	virtual const std::string_view GetName() override
	{
		return m_processor.m_name;
	}

	virtual const double GetFrequency() override
	{
		return m_processor.m_freqency;
	}

	virtual const uint32_t GetCoreNum() override
	{
		return m_processor.m_coreNum;
	}

	virtual const uint32_t GetCacheSize(const CpuCacheLevel& lv) override
	{
		switch (lv)
		{
		case CpuCacheLevel::kL1:
		{
			return m_processor.m_l1Cache.m_size;
		}
			break;
		
		case CpuCacheLevel::kL2:
		{
			return m_processor.m_l2Cache.m_size;
		}
			break;
		
		case CpuCacheLevel::kL3:
		{
			return m_processor.m_l3Cache.m_size;
		}
			break;
		default:
		{
			LOG_ERROR("Please enter proper cache level");
			return -1;
			break;
		}
		}
	}
	
	virtual const uint32_t GetCacheLineSize(const CpuCacheLevel& lv) override
	{
		switch (lv)
		{
		case CpuCacheLevel::kL1:
		{
			return m_processor.m_l1Cache.m_lineSize;
		}
		break;

		case CpuCacheLevel::kL2:
		{
			return m_processor.m_l2Cache.m_lineSize;
		}
		break;

		case CpuCacheLevel::kL3:
		{
			return m_processor.m_l3Cache.m_lineSize;
		}
		break;
		default:
		{
			LOG_ERROR("Please enter proper cache level");
			return -1;
			break;
		}
		}
	}


private:
	bool ReadProcessorName()
	{
		LOG_INFO("============================================================");
		LOG_INFO("\t\t\tProcessor");
		LOG_INFO("============================================================");
		LOG_INFO("Processor Name\t\t:   ", false);

		wchar_t cpuInfo[64] = { 0 };
		HKEY hKey;
		DWORD size = sizeof(cpuInfo);
		
		// Get data from register.
		if (RegOpenKeyExW(HKEY_LOCAL_MACHINE, L"Hardware\\Description\\System\\CentralProcessor\\0", 0, KEY_QUERY_VALUE, &hKey) != ERROR_SUCCESS)
			return false;

		if (RegQueryValueExW(hKey, L"ProcessorNameString", nullptr, nullptr, (LPBYTE)cpuInfo, &size) == ERROR_FILE_NOT_FOUND)
			return false;

		RegCloseKey(hKey);
		m_processor.m_name = WStrToStr(cpuInfo);
		LOG_INFO(m_processor.m_name.c_str());

		return m_processor.m_name.empty();
	}

	bool ReadProcessorType(const SYSTEM_INFO& kSystemInfo)
	{
		LOG_INFO("Processor Type\t\t:   ", false);
		m_processor.m_type = static_cast<uint32_t>(kSystemInfo.dwProcessorType);
		LOG_INFO(std::to_string(m_processor.m_type));

		return (m_processor.m_type > 0);
	}

	bool ReadProcessorFrequency(const SYSTEM_INFO& kSystemInfo)
	{
		LOG_INFO("Number of Core\t\t:   ", false);
		m_processor.m_coreNum = static_cast<size_t>(kSystemInfo.dwNumberOfProcessors);
		LOG_INFO(std::to_string(m_processor.m_coreNum));

		return (m_processor.m_coreNum > 0);
	}

	bool ReadProcessorCache()
	{
		// https://msdn.microsoft.com/en-us/library/windows/desktop/ms683194(v=vs.85).aspx
		DWORD bufferSize = 0;
		if (GetLogicalProcessorInformation(0, &bufferSize))
			return false;

		std::vector<SYSTEM_LOGICAL_PROCESSOR_INFORMATION> buffer(bufferSize / sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION));

		if (!GetLogicalProcessorInformation(buffer.data(), &bufferSize))
			return false;
			   
		for (int i = 0; i != buffer.size(); ++i)
		{
			if (buffer[i].Relationship == RelationCache)
			{
				switch (buffer[i].Cache.Level)
				{
				case 1:
				{
					m_processor.m_l1Cache.m_size = buffer[i].Cache.Size;
					m_processor.m_l1Cache.m_lineSize = buffer[i].Cache.LineSize;
				}
					break;
				case 2:
				{
					m_processor.m_l2Cache.m_size = buffer[i].Cache.Size;
					m_processor.m_l2Cache.m_lineSize = buffer[i].Cache.LineSize;

				}
					break;
				case 3:
				{
					m_processor.m_l3Cache.m_size = buffer[i].Cache.Size;
					m_processor.m_l3Cache.m_lineSize = buffer[i].Cache.LineSize;
				}
					break;
				default:
					break;
				}
			}
		}

		return true;
	}
};

class WindowsStorageInfoReader : public IStorageInfoReader
{
private:
	uint64_t m_availableSpace;
	uint64_t m_totalSpace;
	uint64_t m_freeSpace;

public:
	WindowsStorageInfoReader()
		: m_availableSpace(0)
		, m_totalSpace(0)
		, m_freeSpace(0)
	{
	}

	virtual ~WindowsStorageInfoReader() override
	{
	}

	virtual bool CheckStorageStatus() override
	{
		LOG_INFO("============================================================");
		LOG_INFO("\t\t\tStorage");
		LOG_INFO("============================================================");
	
		return ReadStorageSize();
	}

	virtual uint64_t GetAvailableSpace() override
	{
		return m_availableSpace;
	}

	virtual uint64_t GetTotalSpace() override
	{
		return m_totalSpace;
	}

	virtual uint64_t GetFreeStorageSpace() override
	{
		return m_freeSpace;
	}

private:
	bool ReadStorageSize()
	{
		ULARGE_INTEGER avail;
		ULARGE_INTEGER total;
		ULARGE_INTEGER free;

		avail.QuadPart = 0;
		total.QuadPart = 0;
		free.QuadPart = 0;

		if (!GetDiskFreeSpaceEx(TEXT("C:\\"), &avail, &total, &free))
			return false;

		m_availableSpace = avail.QuadPart;
		m_totalSpace = total.QuadPart;
		m_freeSpace = free.QuadPart;

		std::string str;
		str += "Total available for user-caller\t\t:   " + std::to_string(static_cast<int>(avail.QuadPart >> 30)) + "GB";
		LOG_INFO(str.c_str());
		str.clear();

		str += "Total Space\t\t\t\t:   " + std::to_string(static_cast<int>(total.QuadPart >> 30)) + "GB";
		LOG_INFO(str.c_str());
		str.clear();

		str += "Total Free Space\t\t\t:   " + std::to_string(static_cast<int>(free.QuadPart >> 30)) + "GB\n";
		LOG_INFO(str.c_str());
		str.clear();

		return true;
	}
};

class WindowsGpuInfoReader : public IGpuInfoReader
{
public:
	WindowsGpuInfoReader()
	{
	}
	virtual ~WindowsGpuInfoReader() override
	{
	}

	virtual bool CheckGpuStatus() override
	{
		LOG_INFO("============================================================");
		LOG_INFO("\t\t\tGraphic Card");
		LOG_INFO("============================================================");

		return ReadGpuInfo();
	}

private:
	bool ReadGpuInfo()
	{
		// Key of register.
		HKEY keyVideo;

		std::wstring rootPath = L"SYSTEM\\CurrentControlSet\\Control\\Video\\";
		std::wstring subDir = L"\\0000";

		// Query all subkeys under "SYSTEMS\\CurrentControlSet\\Enum" and save them to keyEnum
		if (RegOpenKeyExW(HKEY_LOCAL_MACHINE, rootPath.data(), 0, KEY_READ, &keyVideo) != ERROR_SUCCESS)
			return false;

		for (size_t i = 0;; ++i)
		{
			std::wstring path = rootPath;

			DWORD size = 512;
			wchar_t dir[512] = { 0 };
			if (RegEnumKeyExW(keyVideo, i, dir, &size, nullptr, nullptr, nullptr, nullptr) == ERROR_NO_MORE_ITEMS)
				break;

			path += (dir + subDir);

			HKEY subKey;

			if (RegOpenKeyExW(HKEY_LOCAL_MACHINE, path.data(), 0, KEY_READ, &subKey) != ERROR_SUCCESS)
				break;

			std::string log;
			log += "========= Video Card " + std::to_string(i + 1) + " =========";
			LOG_INFO(log.c_str());
			log.clear();

			// --- GPU's provider ---
			log += "Provider\t\t: " + ReadInfoFromRegister(subKey, L"ProviderName", size);
			LOG_INFO(log.c_str());
			log.clear();

			// --- GPU's name ---
			log += "GPU \t\t\t: " + ReadInfoFromRegister(subKey, L"HardwareInformation.ChipType", size);
			LOG_INFO(log.c_str());
			log.clear();

			// --- GPU'S driver version ---
			log += "Driver Ver  \t: " + ReadInfoFromRegister(subKey, L"DriverVersion", size) + "\n";
			LOG_INFO(log.c_str());
			log.clear();
		}

		return true;
	}

	std::string ReadInfoFromRegister(const HKEY& kSubKey, const LPCWSTR& kName, DWORD& kSize)
	{
		//auto readStrFromRegister = [&type](HKEY subKey, LPCWSTR name, DWORD size)->std::string
		TCHAR data[512] = { 0 };
		DWORD type = 0;

		if (RegQueryValueExW(kSubKey, kName, nullptr, &type, (LPBYTE)(data), &kSize) == ERROR_FILE_NOT_FOUND)
		{
			RegCloseKey(kSubKey);
			return "N/A";
		}

		return std::string(std::begin(data), std::end(data));
	}
};

class WindowsMemoryInfoReader : public IMemoryInfoReader
{
private:
	uint32_t m_inUsed;
	uint32_t m_totalPhysical;
	uint32_t m_freePhysical;
	uint32_t m_totalVirtual;
	uint32_t m_freeVirtual;

public:
	WindowsMemoryInfoReader()
	{
	}

	virtual ~WindowsMemoryInfoReader() override
	{
	}

	virtual bool CheckMemoryStatus() override
	{
		LOG_INFO("============================================================");
		LOG_INFO("\t\t\tSystem Memory");
		LOG_INFO("============================================================");

		return ReadMemoryInfo();
	}

	virtual uint32_t GetUsedMemorySize() override
	{
		return m_inUsed;
	}

	virtual uint32_t GetFreePhyscialSize() override
	{
		return m_freePhysical;
	}

	virtual uint32_t GetFreeVirtualSize() override
	{
		return m_freeVirtual;
	}

private:
	bool ReadMemoryInfo()
	{
		MEMORYSTATUSEX mem = { 0 };
		mem.dwLength = sizeof(mem);

		std::string log;
		if (int status = GlobalMemoryStatusEx(&mem);
			status == 0)
		{
			log += ("Failed to read memory status " + GetLastError());
			LOG_FATAL(log.c_str());
			return false;
		}

		log += "Memory in use\t\t:   " + std::to_string(mem.dwMemoryLoad) + " percent";
		m_inUsed = static_cast<uint32_t>(mem.dwMemoryLoad);
		LOG_INFO(log.c_str());
		log.clear();

		log += "Total physical memory\t:   " + std::to_string(mem.ullTotalPhys) + " bytes";
		m_totalPhysical = static_cast<uint32_t>(mem.ullTotalPhys);
		LOG_INFO(log.c_str());
		log.clear();

		log += "Free physical memory\t:   " + std::to_string(mem.ullAvailPhys) + " bytes";
		m_freePhysical = static_cast<uint32_t>(mem.ullAvailPhys);
		LOG_INFO(log.c_str());
		log.clear();

		log += "Total virtual memory\t:   " + std::to_string(mem.ullTotalVirtual) + " bytes";
		m_totalVirtual = static_cast<uint32_t>(mem.ullTotalVirtual);
		LOG_INFO(log.c_str());
		log.clear();

		log += "Free virtual memory\t:   " + std::to_string(mem.ullAvailVirtual) + " bytes\n";
		m_freeVirtual = static_cast<uint32_t>(mem.ullAvailVirtual);
		LOG_INFO(log.c_str());
		log.clear();
		
		return true;
	}
};
#endif

/********************************************
*											*
*	       Reader Factory Functions         * 
*											*
*********************************************/
std::unique_ptr<IProcessorInfoReader> IProcessorInfoReader::Create()
{
#ifdef _WIN32
	return std::make_unique<WindowsProcessorInfoReader>();
#else
	return std::make_unique<IProcessorInfoReader>();
#endif
}

std::unique_ptr<IStorageInfoReader> IStorageInfoReader::Create()
{
#ifdef _WIN32
	return std::make_unique<WindowsStorageInfoReader>();
#else
	return std::make_unique<IStorageInfoReader>();
#endif
}

std::unique_ptr<IGpuInfoReader> IGpuInfoReader::Create()
{
#ifdef _WIN32
	return std::make_unique<WindowsGpuInfoReader>();
#else
	return std::make_unique<IGpuInfoReader>();
#endif
}

std::unique_ptr<IMemoryInfoReader> IMemoryInfoReader::Create()
{
#ifdef _WIN32
	return std::make_unique<WindowsMemoryInfoReader>();
#else
	return std::make_unique<IMemoryInfoReader>();
#endif
}
