// 3d vector class for csgo vector and view angle manipulations

#pragma once
#include <numbers>
#include <cmath>

class Vector3
{
    float x, y, z;

public:
    constexpr Vector3(const float x = 0.0f, const float y = 0.0f, const float z = 0.0f)
        : x(x), y(y), z(z) { }

    // common operator overloads
    constexpr Vector3 operator-(const Vector3& other) const noexcept
    {
        return Vector3{ x - other.x, y - other.y, z - other.z };
    }

    constexpr Vector3 operator+(const Vector3& other) const noexcept
    {
        return Vector3{ x + other.x, y + other.y, z + other.z };
    }

    constexpr Vector3 operator/(const float factor) const noexcept
    {
        return Vector3{ x / factor, y / factor, z / factor };
    }

    constexpr Vector3 operator*(const float factor) const noexcept
    {
        return Vector3{ x * factor, y * factor, z * factor };
    }

    constexpr Vector3 toAngle() const noexcept
    {
        return Vector3{ 
            // converting the vector to a view angle (and to radians)
            std::atan2(-z, std::hypot(x, y)) * (180.0f / std::numbers::pi_v<float>),
            std::atan2(y, z) * (180.0f / std::numbers::pi_v<float>),
            0.0f
        };
    }

    constexpr bool IsZero() const noexcept
    {
        return x == 0.f && y == 0.f && z == 0.f;
    }
};