#include "Entity.hpp"

VectorF Entity::DecodeVelocity(short x, short y, short z)
{
    const float ticksPerSecond = 20;
    const double velMod = 1 / 8000.0;
    return VectorF(x * velMod * ticksPerSecond, y*velMod*ticksPerSecond, z*velMod*ticksPerSecond);
}
