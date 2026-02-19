#pragma once
#include <DirectXMath.h>

using namespace DirectX;

namespace Utils
{
    /* =========================
       XMFLOAT2
       ========================= */

    inline XMFLOAT2 operator+(const XMFLOAT2& a, const XMFLOAT2& b)
    {
        return { a.x + b.x, a.y + b.y };
    }

    inline XMFLOAT2 operator-(const XMFLOAT2& a, const XMFLOAT2& b)
    {
        return { a.x - b.x, a.y - b.y };
    }

    inline XMFLOAT2 operator*(const XMFLOAT2& a, const XMFLOAT2& b)
    {
        return { a.x * b.x, a.y * b.y };
    }

    inline XMFLOAT2 operator*(const XMFLOAT2& v, float s)
    {
        return { v.x * s, v.y * s };
    }

    inline XMFLOAT2 operator/(const XMFLOAT2& v, float s)
    {
        return { v.x / s, v.y / s };
    }

    inline XMFLOAT2& operator+=(XMFLOAT2& a, const XMFLOAT2& b)
    {
        a = a + b;
        return a;
    }

    /* =========================
       XMFLOAT3
       ========================= */

    inline XMFLOAT3 operator+(const XMFLOAT3& a, const XMFLOAT3& b)
    {
        return { a.x + b.x, a.y + b.y, a.z + b.z };
    }

    inline XMFLOAT3 operator-(const XMFLOAT3& a, const XMFLOAT3& b)
    {
        return { a.x - b.x, a.y - b.y, a.z - b.z };
    }

    inline XMFLOAT3 operator*(const XMFLOAT3& a, const XMFLOAT3& b)
    {
        return { a.x * b.x, a.y * b.y, a.z * b.z };
    }

    inline XMFLOAT3 operator*(const XMFLOAT3& v, float s)
    {
        return { v.x * s, v.y * s, v.z * s };
    }

    inline XMFLOAT3 operator/(const XMFLOAT3& v, float s)
    {
        return { v.x / s, v.y / s, v.z / s };
    }

    inline XMFLOAT3& operator+=(XMFLOAT3& a, const XMFLOAT3& b)
    {
        a = a + b;
        return a;
    }
    
    inline XMFLOAT3& operator-=(XMFLOAT3& a, const XMFLOAT3& b)
    {
        a = a - b;
        return a;
    }

    inline XMFLOAT3& operator*=(XMFLOAT3& a, float s)
    {
        a = a * s;
        return a;
    }
    
    inline XMFLOAT3& operator/=(XMFLOAT3& a, float s)
    {
        a = a / s;
        return a;
    }

    /* =========================
       XMFLOAT4
       ========================= */

    inline XMFLOAT4 operator+(const XMFLOAT4& a, const XMFLOAT4& b)
    {
        return { a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w };
    }

    inline XMFLOAT4 operator*(const XMFLOAT4& v, float s)
    {
        return { v.x * s, v.y * s, v.z * s, v.w * s };
    }

    /* =========================
       Fonctions utiles
       ========================= */

    inline float Length(const XMFLOAT3& v)
    {
        return sqrtf(v.x * v.x + v.y * v.y + v.z * v.z);
    }

    inline XMFLOAT3 Normalize(const XMFLOAT3& v)
    {
        float len = Length(v);
        return (len > 0.0f) ? v / len : XMFLOAT3{ 0,0,0 };
    }

    inline float Dot(const XMFLOAT3& a, const XMFLOAT3& b)
    {
        return a.x * b.x + a.y * b.y + a.z * b.z;
    }

    inline XMFLOAT3 Cross(const XMFLOAT3& a, const XMFLOAT3& b)
    {
        return
        {
            a.y * b.z - a.z * b.y,
            a.z * b.x - a.x * b.z,
            a.x * b.y - a.y * b.x
        };
    }

    inline float Distance(const XMFLOAT3& a, const XMFLOAT3& b)
    {
        return sqrtf(pow(a.x - b.x, 2) + pow(a.y - b.y, 2)+ pow(a.z - b.z, 2));
    }

    inline float Distance(const XMFLOAT2& a, const XMFLOAT2& b)
    {
        return sqrtf(pow(a.x - b.x, 2) + pow(a.y - b.y, 2));
    }
}
