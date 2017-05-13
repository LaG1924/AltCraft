#include "PositionF.hpp"

PositionF::PositionF(double x, double z, double y) : m_x(x), m_y(y), m_z(z) {

}

PositionF::~PositionF() {

}

double PositionF::GetX() const {
    return m_x;
}

double PositionF::GetY() const {
    return m_y;
}

double PositionF::GetZ() const {
    return m_z;
}

void PositionF::SetX(double x) {
    m_x = x;
}

void PositionF::SetY(double y) {
    m_y = y;
}

void PositionF::setZ(double z) {
    m_z = z;
}

bool PositionF::operator==(const PositionF &other) const {
    return other.m_x == m_x && other.m_z == m_z && other.m_y == other.m_y;
}

PositionF &PositionF::operator=(const PositionF &other) {
    m_y = other.m_y;
    m_z = other.m_z;
    m_x = other.m_x;
    return *this;
}

PositionF::PositionF(const PositionF &other) {
    m_y = other.m_y;
    m_z = other.m_z;
    m_x = other.m_x;
}
