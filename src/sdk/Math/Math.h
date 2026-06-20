// Vec3.h
#pragma once

namespace Math
{
    class Vec3
    {
    public:
        Vec3() = default;
        Vec3(float x, float y, float z);

        Vec3 operator+(const Vec3& arg) const;
        Vec3& operator+=(const Vec3& arg);

    private:
        float m_x = 0, m_y = 0, m_z = 0;
    };
}

#include "Vec3.inl"
