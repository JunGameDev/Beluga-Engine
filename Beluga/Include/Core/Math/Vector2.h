#pragma once
#include "MathHelper.h"

namespace Bel
{
    template <typename Type>
    class Vector2
    {
        static_assert(std::is_trivial<Type>::value, "Type must be a trivial type");
    public:
        Type m_x;
        Type m_y;

        static Vector2<Type> s_Zero;

    public:
        constexpr Vector2() : m_x(0), m_y(0) 
        {
        }
        constexpr Vector2(std::nullptr_t) 
        {
        }
        constexpr Vector2(const Vector2<Type>& vec) 
            : m_x(vec.m_x)
            , m_y(vec.m_y) 
        {
        }
        constexpr Vector2(Type x, Type y) 
            : m_x(x)
            , m_y(y) 
        {
        }

        template <typename OtherType>
        constexpr Vector2(const Vector2<OtherType>& right)
            : m_x(static_cast<Type>(right.m_x))
            , m_y(static_cast<Type>(right.m_y))
        {
        }

        template <typename OtherType>
        constexpr Vector2(OtherType x, OtherType y)
            : m_x(static_cast<Type>(x))
            , m_y(static_cast<Type>(y))
        {
        }

        Vector2<Type>& operator=(const Vector2<Type>& other) 
        { 
            m_x = other.m_x; 
            m_y = other.m_y;
            return (*this);
        }
        Vector2<Type>& operator=(const Vector2<Type>* pOther) 
        { 
            m_x = pOther->m_x; 
            m_y = pOther->m_y;
            return (*this);
        }
        Vector2<Type>& operator*=(const Vector2<Type>& other) 
        { 
            m_x *= other.m_x; 
            m_y *= other.m_y; 
            return (*this);
        }
        Vector2<Type>& operator*=(Type scalar) 
        { 
            m_x = scalar; 
            m_y = scalar;
            return (*this);
        }
        Vector2<Type>& operator+=(const Vector2<Type>& other) 
        { 
            m_x += other.m_x; 
            m_y += other.m_y;
            return (*this);
        }       
        Vector2<Type>& operator-=(const Vector2<Type>& other) 
        { 
            m_x -= other.m_x; 
            m_y -= other.m_y;
            return (*this);
        }

        constexpr bool operator==(const Vector2<Type>& other) const 
        { 
            return (m_x == other.m_x) && (m_y == other.m_y);
        }
        constexpr bool operator!=(const Vector2<Type>& other) const
        {
            return (m_x != other.m_x) || (m_y != other.m_y);
        }
        constexpr Vector2<Type> operator-(const Vector2<Type>& other) const
        {
            return Vector2<Type>(-m_x, -m_y);
        }

        template <typename OtherType>
        Vector2<Type>& operator=(const Vector2<OtherType>& other)
        {
            m_x = static_cast<Type>(other.m_x);
            m_y = static_cast<Type>(other.m_y);
            return (*this);
        }

        void Set(Type x, Type y) 
        { 
            m_x = x;
            m_y = y;
        }

        constexpr float LengthSquared() const 
        { 
            return static_cast<float>((m_x * m_x) + (m_y * m_y));
        }
        constexpr float Length() const { return SquareRoot(LengthSquared()); }

        constexpr bool IsZero() const { return (m_x == 0) && (m_y == 0); }
        constexpr Type Dot(const Vector2<Type>& other) const
        {
            return (m_x * other.m_x) + (m_y * other.m_y);
        }
        constexpr Type DistanceTo(const Vector2<Type>& other) const
        {
            return ((*this - other).Length());
        }
        
        template <typename FuncType = Type>
        typename std::enable_if_t<std::is_floating_point<FuncType>::value, void> Normalize()
        {
            if (IsZero()) return;
            const float kLength = Length();
            m_x /= kLength;
            m_y /= kLength;
        }
    };

    template <typename Type>
    Vector2<Type> operator+(const Vector2<Type>& left, const Vector2<Type>& right)
    {
        return Vector2<Type>(left.m_x + right.m_x, left.m_y + right.m_y);
    }

    template <typename Type, typename OtherType>
    Vector2<Type> operator+(const Vector2<Type>& left, const Vector2<OtherType>& right)
    {
        return Vector2<Type>(left.m_x + static_cast<Type>(right.m_x), left.m_y + static_cast<Type>(right.m_y));
    }

    template <typename Type, typename OtherType>
    Vector2<Type> operator+(const Vector2<Type>& left, OtherType scalar)
    {
        return Vector2<Type>(left.m_x + static_cast<Type>(scalar), left.m_y + static_cast<Type>(scalar));
    }

    template <typename Type>
    Vector2<Type> operator-(const Vector2<Type>& left, const Vector2<Type>& right)
    {
        return Vector2<Type>(left.m_x - right.m_x, left.m_y - right.m_y);
    }

    template <typename Type, typename OtherType>
    Vector2<Type> operator-(const Vector2<Type>& left, const Vector2<OtherType>& right)
    {
        return Vector2<Type>(left.m_x - static_cast<Type>(right.m_x), left.m_y - static_cast<Type>(right.m_y));
    }

    template <typename Type, typename OtherType>
    Vector2<Type> operator-(const Vector2<Type>& left, OtherType scalar)
    {
        return Vector2<Type>(left.m_x - scalar, left.m_y - scalar);
    }

    template <typename Type>
    Vector2<Type> operator*(const Vector2<Type>& left, const Vector2<Type>& right)
    {
        return Vector2<Type>(left.m_x * right.m_x, left.m_y * right.m_y);
    }

    template <typename Type, typename OtherType>
    Vector2<Type> operator*(const Vector2<Type>& left, const Vector2<OtherType>& right)
    {
        return Vector2<Type>(left.m_x * static_cast<Type>(right.m_x), left.m_y * static_cast<Type>(right.m_y));
    }

    template <typename Type, typename OtherType>
    Vector2<Type> operator*(const Vector2<Type>& left, OtherType scalar)
    {
        return Vector2<Type>(left.m_x * static_cast<Type>(scalar), left.m_y * static_cast<Type>(scalar));
    }

    template <typename Type>
    Vector2<Type> operator/(const Vector2<Type>& left, const Vector2<Type>& right)
    {
        return Vector2<Type>(left.m_x / right.m_x, left.m_y / right.m_y);
    }

    template <typename Type, typename OtherType>
    Vector2<Type> operator/(const Vector2<Type>& left, const Vector2<OtherType>& right)
    {
        return Vector2<Type>(left.m_x / static_cast<Type>(right.m_x), left.m_y / static_cast<Type>(right.m_y));
    }

    template <typename Type, typename OtherType>
    Vector2<Type> operator/(const Vector2<Type>& left, OtherType scalar)
    {
        return Vector2<Type>(left.m_x / static_cast<Type>(scalar), left.m_y / static_cast<Type>(scalar));
    }

    template<typename Type>
    constexpr float Distance(const Vector2<Type>& left, const Vector2<Type>& right)
    {
        return (left - right).Length();
    }

    template<typename Type, typename OtherType>
    constexpr float Distance(const Vector2<Type>& left, const Vector2<OtherType>& right)
    {
        return (left - right).Length();
    }

    template<typename Type>
    Vector2<Type> Vector2<Type>::s_Zero = Vector2<Type>();
}


