#pragma once

#include <ostream>
#include <cmath>
#include <tuple>

template<class T>
class Vector3 {
    T x, y, z;
public:
    Vector3(T X = 0, T Y = 0, T Z = 0) : x(X), y(Y), z(Z) {}

    Vector3(const Vector3 &rhs) : x(rhs.x), y(rhs.y), z(rhs.z) {}

    ~Vector3() = default;

    void SetX(T X) { x = X; }

    void SetY(T Y) { y = Y; }

    void setZ(T Z) { z = Z; }

    T GetX() const { return x; }

    T GetY() const { return y; }

    T GetZ() const { return z; }

    double GetDistance() const { return std::sqrt(std::pow(x, 2) + std::pow(y, 2) + std::pow(z, 2)); }

    void swap(Vector3 &rhs){
        std::swap(x,rhs.x);
        std::swap(y,rhs.y);
        std::swap(z,rhs.z);
    }

    Vector3 &operator=(Vector3 rhs) {
        rhs.swap(*this);
        return *this;
    }

    Vector3 operator*(T rhs) const {
        return Vector3<T>(
                x * rhs,
                y * rhs,
                z * rhs
        );
    }

    Vector3 operator/(T rhs) const {
        return Vector3<T>(
                x / rhs,
                y / rhs,
                z / rhs
        );
    }

    Vector3 operator+(const Vector3 &rhs) const {
        return Vector3<T>(
                x + rhs.x,
                y + rhs.y,
                z + rhs.z
        );
    }

    Vector3 operator-(const Vector3 &rhs) const {
        return Vector3<T>(
                x - rhs.x,
                y - rhs.y,
                z - rhs.z
        );
    }

    Vector3 operator*(const Vector3 &rhs) const {
        return Vector3<T>(
                x * rhs.x,
                y * rhs.y,
                z * rhs.z
        );
    }

    Vector3 operator/(const Vector3 &rhs) const {
        return Vector3<T>(
                x / rhs.x,
                y / rhs.y,
                z / rhs.z
        );
    }

    bool operator==(const Vector3 &rhs) const {
        return (x == rhs.x && y == rhs.y && z == rhs.z);
    }

    bool operator!=(const Vector3 &rhs) const {
        return !(*this == rhs);
    }

    bool operator<(const Vector3 &rhs) const {
        //return (x < rhs.x || y < rhs.y ||z z < rhs.z);
        /*if (x < rhs.x)
            return true;
        else if (z < rhs.z)
            return true;
        else if (y < rhs.y)
            return true;
        return false;*/
        return std::tie(x,y,z)<std::tie(rhs.x,rhs.y,rhs.z);
    }


    friend std::ostream &operator<<(std::ostream &os, const Vector3 &vector3) {
        os << vector3.x << ", " << vector3.y << ", " << vector3.z;
        return os;
    }
};

typedef Vector3<double> VectorF;
typedef Vector3<signed long long> Vector;