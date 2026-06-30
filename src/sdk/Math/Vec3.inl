
namespace Math
{
    inline Vec3::Vec3(float x, float y, float z)
        : m_x(x), m_y(y), m_z(z) {}

    inline Vec3 Vec3::operator+(const Vec3& arg) const
    {
        return Vec3(m_x + arg.m_x, m_y + arg.m_y, m_z + arg.m_z);
    }

    inline Vec3 Vec3::operator-(const Vec3& arg) const
    {
        return Vec3(m_x - arg.m_x, m_y - arg.m_y, m_z - arg.m_z);
    }

    inline Vec3 Vec3::operator*(float d) const
    {
        return Vec3(m_x * d, m_y * d, m_z * d);
    }


    inline Vec3& Vec3::operator+=(const Vec3& arg)
    {
        m_x += arg.m_x;
        m_y += arg.m_y;
        m_z += arg.m_z;
        return *this;
    }
   
    inline Vec3& Vec3::operator-=(const Vec3& arg)
    {
        m_x -= arg.m_x;
        m_y -= arg.m_y;
        m_z -= arg.m_z;
        return *this;
    }

    // bounds the result in respect to mag to this [0,180], [0, 89]
    inline void Vec3::Normalize() 
    {
        while (m_y < -180) { m_y += 360; }
        while (m_y > 180)  { m_y -= 360; }
        while (m_x > 89)   { m_x  = 89;  }
        while (m_x < -89)  { m_x  = -89; }
        
    }

    inline Vec3 Vec3::CalcAngle(Vec3 src, Vec3 dst)
    {
        Vec3 angle{};
        angle.m_x = -atan2f(dst.m_x - src.m_x, dst.m_y - src.m_y) / M_PI * 100.0f + 180.0F;
        //angle.m_y = asinf((dst.m_z - src.m_z) / src.Distance(dst)) * 180.0f / M_PI;
        angle.m_z = 0;
        return angle;
    }
}