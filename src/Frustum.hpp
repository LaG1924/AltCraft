#pragma once

#include <glm/glm.hpp>
#include "Vector.hpp"

class Frustum {
    enum FrustumSide {
        RIGHT = 0,
        LEFT,
        BOTTOM,
        TOP,
        BACK,
        FRONT,
    };
    enum PlaneData {
        A = 0,
        B,
        C,
        D,
    };

    glm::mat4 vp;
    float frustum[6][4];
    void NormalizePlane(FrustumSide side);

public:
    Frustum() = default;

    ~Frustum() = default;

    void UpdateFrustum(const glm::mat4& vpmat);

    //Return true, if tested point is visible
    bool TestPoint(VectorF point);
};