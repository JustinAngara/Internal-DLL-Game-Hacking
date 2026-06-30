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
        Vec3 operator*(float d) const;
        Vec3 operator-(const Vec3& arg) const;

        Vec3& operator=(const Vec3& arg);
        Vec3& operator+=(const Vec3& arg);
        Vec3& operator-=(const Vec3& arg);
     
        // mutator
        void Normalize();
            
        // non mutators
        void Subtract();
        void Divide();
        void Magnitude();
        void DotProduct();
    private:
        float m_x = 0, m_y = 0, m_z = 0;
    };
}

#include "Vec3.inl"
