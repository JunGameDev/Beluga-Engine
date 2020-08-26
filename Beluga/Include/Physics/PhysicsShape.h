#pragma once
#include <memory>
#include "Parshing/tinyxml2.h"

namespace Bel
{
    class IFixtureDef
    {
    public:
        virtual ~IFixtureDef() = 0 {}
        virtual void* GetFixture() = 0;

        static std::unique_ptr<IFixtureDef> Create();
    };

    class IPhysicalShape
    {
    public:
        enum class Type
        {
            kBox,
            kCircle,
            kPolygon,
            kChain,
            kNone,
            kCount
        };

    public:
        IPhysicalShape() {}
        virtual ~IPhysicalShape() = 0 {}
        virtual void Initialize(tinyxml2::XMLElement* pData) = 0;
        virtual void SetThisToFixture(IFixtureDef* pFixture) = 0;

        virtual float GetDensity()  = 0;
        virtual float GetWidth()    = 0;
        virtual float GetHeight()   = 0;
        virtual float GetFriction() = 0;
        virtual float GetRadius() = 0;
        virtual Vector2<float> GetCenter() = 0;

        static std::unique_ptr<IPhysicalShape> Create(Type& type);
    };
}

