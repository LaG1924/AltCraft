#pragma once

#include "Utility.hpp"
#include "Vector.hpp"

struct Entity {
    Uuid uuid;
    VectorF pos;
    VectorF vel;
    unsigned int entityId;
    double yaw;
    double pitch;
    static VectorF DecodeVelocity(short x, short y, short z);
};