#pragma once

#include <ostream>
#include <cmath>

#include <glm/vec3.hpp>

template<class T>
struct Vector3 {
    T x, y, z;

    Vector3(T X = 0, T Y = 0, T Z = 0) : x(X), y(Y), z(Z) {}

    Vector3(const Vector3 &rhs) : x(rhs.x), y(rhs.y), z(rhs.z) {}

    ~Vector3() = default;

    double GetLength() const { return std::sqrt(std::pow(x, 2) + std::pow(y, 2) + std::pow(z, 2)); }

    operator glm::vec3() const {
        return glm::vec3(x, y, z);
    }

    glm::vec3 glm() const {
        return (glm::vec3)(*this);
    }

    void swap(Vector3 &rhs) noexcept {
        std::swap(x, rhs.x);
        std::swap(y, rhs.y);
        std::swap(z, rhs.z);
    }

    T dot(const Vector3 &rhs) const {
        return x*rhs.x + y*rhs.y + z*rhs.z;
    }

    double cosBetween(const Vector3<T> &rhs) const {
        return dot(rhs) / GetLength() / rhs.GetLength();
    }

    Vector3<double> normalize() {
        auto length = GetLength();

        return Vector3<double> (
                x / length,
                y / length,
                z / length
        );
    }

    Vector3 &operator=(Vector3 rhs) noexcept {
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

    Vector3 operator-() const {
        return Vector3<T> (
                -x,
                -y,
                -z
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
        if (x < rhs.x)
            return true;
        if (rhs.x < x)
            return false;
        if (y < rhs.y)
            return true;
        if (rhs.y < y)
            return false;
        return z < rhs.z;
    }


    friend std::ostream &operator<<(std::ostream &os, const Vector3 &vector3) {
        os << vector3.x << " " << vector3.y << " " << vector3.z;
        return os;
    }
};

using VectorF = Vector3<double>;
using Vector = Vector3<signed long long>;