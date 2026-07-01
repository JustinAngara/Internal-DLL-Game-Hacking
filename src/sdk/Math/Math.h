// Vec3.h
#pragma once

#include <math.h>

#define M_PI 3.14159265

namespace Math
{
    class Vec3
    {
    public:
        Vec3() = default;
        Vec3(float x, float y, float z);

        // operations
        Vec3 operator+(const Vec3& arg) const;
        Vec3 operator*(float d) const;
        Vec3 operator-(const Vec3& arg) const;


        Vec3& operator=(const Vec3& arg) = default;
        Vec3& operator+=(const Vec3& arg);
        Vec3& operator-=(const Vec3& arg);
     

        // mutator
        void Normalize();
            
        // non mutators
        void Subtract();
        void Divide();
        void Magnitude();
        void DotProduct();
        Vec3 CalcAngle(Vec3 src, Vec3 dst); // do implementation
    private:
        float m_x = 0, m_y = 0, m_z = 0;
    };

}

#include "Vec3.inl"
