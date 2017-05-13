#include <cmath>
#include "PositionI.hpp"

PositionI::PositionI(int x, int z, int y) : m_x(x), m_y(y), m_z(z) {

}

PositionI::~PositionI() {

}

int PositionI::GetX() const {
    return m_x;
}

int PositionI::GetY() const {
    return m_y;
}

int PositionI::GetZ() const {
    return m_z;
}

void PositionI::SetX(int x) {
    m_x = x;
}

void PositionI::SetY(int y) {
    m_y = y;
}

void PositionI::setZ(int z) {
    m_z = z;
}

bool PositionI::operator==(const PositionI &other) const {
    return other.m_x == m_x && other.m_z == m_z && other.m_y == other.m_y;
}

PositionI &PositionI::operator=(const PositionI &other) {
    m_y = other.m_y;
    m_z = other.m_z;
    m_x = other.m_x;
    return *this;
}

PositionI::PositionI(const PositionI &other) {
    m_y = other.m_y;
    m_z = other.m_z;
    m_x = other.m_x;
}

PositionI::PositionI() : m_x(0), m_y(0), m_z(0) {

}

bool PositionI::operator<(const PositionI &rhs) const {
    if (m_x < rhs.m_x)
        return true;
    if (rhs.m_x < m_x)
        return false;
    if (m_y < rhs.m_y)
        return true;
    if (rhs.m_y < m_y)
        return false;
    return m_z < rhs.m_z;
}

bool PositionI::operator>(const PositionI &rhs) const {
    return rhs < *this;
}

bool PositionI::operator<=(const PositionI &rhs) const {
    return !(rhs < *this);
}

bool PositionI::operator>=(const PositionI &rhs) const {
    return !(*this < rhs);
}

PositionI PositionI::operator-(const PositionI &other) const {
    return PositionI(
            m_x - other.m_x,
            m_z - other.m_z,
            m_y - other.m_y
    );
}

double PositionI::GetDistance() {
    return (std::sqrt(std::pow(m_x, 2) + std::pow(m_y, 2) + std::pow(m_z, 2)));
}

PositionI PositionI::operator*(int other) const {
    return PositionI(
            m_x * other,
            m_z * other,
            m_y * other
    );
}

PositionI PositionI::operator*(const PositionI &other) const {
    return PositionI(
            m_x * other.m_x,
            m_z * other.m_z,
            m_y * other.m_y
    );
}

PositionI PositionI::operator/(int other) const {
    return PositionI(
            m_x / other,
            m_z / other,
            m_y / other
    );
}

std::ostream &operator<<(std::ostream &os, const PositionI &i) {
    os << "(" << i.m_x << ", " << i.m_y << ", " << i.m_z << ")";
    return os;
}
