#pragma once

class PositionF {
public:
    PositionF(double x, double z, double y);

    ~PositionF();

    double GetX() const;

    double GetY() const;

    double GetZ() const;

    void SetX(double x);

    void SetY(double y);

    void setZ(double z);

    bool operator==(const PositionF &other) const;

    PositionF &operator=(const PositionF &other);

    PositionF(const PositionF &other);

private:
    double m_x, m_y, m_z;
};