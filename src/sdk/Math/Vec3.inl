namespace Math
{
    inline Vec3::Vec3(float x, float y, float z)
        : m_x(x), m_y(y), m_z(z) {}

    inline Vec3 Vec3::operator+(const Vec3& arg) const
    {
        return Vec3(m_x + arg.m_x, m_y + arg.m_y, m_z + arg.m_z);
    }

    inline Vec3& Vec3::operator+=(const Vec3& arg)
    {
        m_x += arg.m_x;
        m_y += arg.m_y;
        m_z += arg.m_z;
        return *this;
    }
}