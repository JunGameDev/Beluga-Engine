#pragma once
#include <math.h>
#include <limits>

namespace Bel
{
    template <typename FloatType>
    constexpr FloatType SquareRootHelper(FloatType x, FloatType curr, FloatType prev)
    {
        return (curr == prev)
            ? curr
            : SquareRootHelper<FloatType>(x, static_cast<FloatType>(0.5) * (curr + x / curr), curr);
    }

    template <typename FloatType>
    constexpr FloatType SquareRoot(FloatType x)
    {
        return x >= 0 && x < std::numeric_limits<FloatType>::infinity()
            ? SquareRootHelper<FloatType>(x, x, 0)
            : std::numeric_limits<FloatType>::quiet_NaN();
    }

    template<class Type>
    constexpr Type Square(Type x)
    {
        return x * x;
    }
}