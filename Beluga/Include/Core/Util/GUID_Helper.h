#pragma once
#include <stdexcept>
#include <string>
#include <cassert>
#include <cstdint>
#include <guiddef.h>
#include <rpc.h>

namespace Bel
{
    //struct Guid
    //{
    //    uint32_t m_data1;
    //    uint16_t m_data2;
    //    uint16_t m_data3;
    //    uint8_t  m_data4[8];
    //};

    constexpr const size_t kShortGuidLength = 36;
    constexpr const size_t kLongGuidLength = 38;

    constexpr int ParseHexDigit(const char c)
    {
        if ('0' <= c && c <= '9')
            return c - '0';
        else if ('a' <= c && c <= 'f')
            return 10 + c - 'a';
        else if ('A' <= c && c <= 'F')
            return 10 + c - 'A';
    }

    template<class Type>
    constexpr Type ParseHex(const char* ptr)
    {
        constexpr size_t digits = sizeof(Type) * 2;
        Type result{};
        for (size_t i = 0; i < digits; ++i)
            result |= ParseHexDigit(ptr[i]) << (4 * (digits - i - 1));
        return result;
    }

    constexpr GUID MakeGuidHelper(const char* begin)
    {
        GUID result{};
        result.Data1 = ParseHex<uint32_t>(begin);
        begin += 8 + 1;
        result.Data2 = ParseHex<uint16_t>(begin);
        begin += 4 + 1;
        result.Data3= ParseHex<uint16_t>(begin);
        begin += 4 + 1;
        result.Data4[0] = ParseHex<uint8_t>(begin);
        begin += 2;
        result.Data4[1] = ParseHex<uint8_t>(begin);
        begin += 2 + 1;

        for (size_t i = 0; i < 6; ++i)
            result.Data4[i + 2] = ParseHex<uint8_t>(begin + i * 2);
        
        return result;
    }

    template<size_t Size>
    constexpr GUID MakeGuid(const char(&str)[Size])
    {
        static_assert(Size == (kLongGuidLength + 1) || Size == (kShortGuidLength + 1), "String GUID of the form {XXXXXXXX-XXXX-XXXX-XXXX-XXXXXXXXXXXX} or XXXXXXXX-XXXX-XXXX-XXXX-XXXXXXXXXXXX is expected");
        return MakeGuidHelper(str + (Size == (kLongGuidLength + 1) ? 1 : 0));
    }
}

namespace std 
{
    template<> struct hash<GUID>
    {
        size_t operator()(const GUID& guid) const noexcept 
        {
            const std::uint64_t* p = reinterpret_cast<const std::uint64_t*>(&guid);
            std::hash<std::uint64_t> hash;
            return hash(p[0]) ^ hash(p[1]);
        }
    };
}