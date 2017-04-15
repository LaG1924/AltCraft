#pragma once

class PositionI {
public:
    PositionI(int x, int z, int y);

    PositionI();

    ~PositionI();

    int GetX() const;

    int GetY() const;

    int GetZ() const;

    void SetX(int x);

    void SetY(int y);

    void setZ(int z);

    bool operator==(const PositionI &other) const;

    PositionI &operator=(const PositionI &other);

    PositionI(const PositionI &other);

    bool operator<(const PositionI &rhs) const;

    bool operator>(const PositionI &rhs) const;

    bool operator<=(const PositionI &rhs) const;

    bool operator>=(const PositionI &rhs) const;

private:
    int m_x, m_y, m_z;
};