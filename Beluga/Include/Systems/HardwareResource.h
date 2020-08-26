// Copyright (c) 2020 Junyoung Kim
#pragma once
#include <string_view>

// Interfaces for reading hardware information.
namespace Bel
{
	enum class CpuCacheLevel
	{
		kL1,
		kL2,
		kL3,
		kCount
	};

	struct CpuCache
	{
		uint32_t m_size;
		uint32_t m_lineSize;
	};

	struct Processor
	{
		std::string m_name;
		uint32_t m_type;
		double m_freqency;
		uint32_t m_coreNum;
		CpuCache m_l1Cache;
		CpuCache m_l2Cache;
		CpuCache m_l3Cache;
	};
	
	class IProcessorInfoReader
	{
	public:
		virtual ~IProcessorInfoReader() = 0 {}

		virtual bool CheckProcessorStatus() = 0;
		virtual const std::string_view GetName() = 0;
		virtual const double		   GetFrequency()  = 0;
		virtual const uint32_t         GetCoreNum() = 0;
		virtual const uint32_t         GetCacheSize(const CpuCacheLevel& lv) = 0;
		virtual const uint32_t         GetCacheLineSize(const CpuCacheLevel& lv) = 0;

		static std::unique_ptr<IProcessorInfoReader> Create();
	};

	class IStorageInfoReader
	{
	public:
		virtual ~IStorageInfoReader() = 0 {}

		virtual bool CheckStorageStatus() = 0;
		virtual uint64_t GetAvailableSpace() = 0;
		virtual uint64_t GetTotalSpace() = 0;
		virtual uint64_t GetFreeStorageSpace() = 0;

		static std::unique_ptr<IStorageInfoReader> Create();
	};

	class IGpuInfoReader
	{
	public:
		virtual ~IGpuInfoReader() = 0 {}
		virtual bool CheckGpuStatus() = 0;

		static std::unique_ptr<IGpuInfoReader> Create();
	};

	class IMemoryInfoReader
	{
	public:
		virtual ~IMemoryInfoReader() = 0 {}
		virtual bool CheckMemoryStatus() = 0;

		virtual uint32_t GetUsedMemorySize()   = 0;
		virtual uint32_t GetFreePhyscialSize() = 0;
		virtual uint32_t GetFreeVirtualSize()  = 0;

		static std::unique_ptr<IMemoryInfoReader> Create();
	};
}