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
}