#include "Entity.hpp"

VectorF Entity::DecodeVelocity(short x, short y, short z)
{
    const float ticksPerSecond = 20;
    const double velMod = 1 / 8000.0;
    return VectorF(x * velMod * ticksPerSecond, y*velMod*ticksPerSecond, z*velMod*ticksPerSecond);
}

VectorF Entity::DecodeDeltaPos(short deltaX, short deltaY, short deltaZ)
{
    const double posMod = 4096.0;
    return VectorF(deltaX / posMod, deltaY / posMod, deltaZ / posMod);
}

Entity CreateObject(ObjectType type)
{
    Entity entity;
    entity.isMob = false;
    switch (type) {
    case ObjectType::Boat:        
        break;
    case ObjectType::ItemStack:
        entity.width = 0.25;
        entity.height = 0.25;
        break;
    case ObjectType::AreaEffectCloud:
        break;
    case ObjectType::Minecart:
        break;
    case ObjectType::ActivatedTNT:
        break;
    case ObjectType::EnderCrystal:
        break;
    case ObjectType::TippedArrow:
        break;
    case ObjectType::Snowball:
        break;
    case ObjectType::Egg:
        break;
    case ObjectType::FireBall:
        break;
    case ObjectType::FireCharge:
        break;
    case ObjectType::ThrownEnderpearl:
        break;
    case ObjectType::WitherSkull:
        break;
    case ObjectType::ShulkerBullet:
        break;
    case ObjectType::LlamaSpit:
        break;
    case ObjectType::FallingObjects:
        break;
    case ObjectType::Itemframes:
        break;
    case ObjectType::EyeOfEnder:
        break;
    case ObjectType::ThrownPotion:
        break;
    case ObjectType::ThrownExpBottle:
        break;
    case ObjectType::FireworkRocket:
        break;
    case ObjectType::LeashKnot:
        break;
    case ObjectType::ArmorStand:
        break;
    case ObjectType::EvocationFangs:
        break;
    case ObjectType::FishingHook:
        break;
    case ObjectType::SpectralArrow:
        break;
    case ObjectType::DragonFireball:
        break;
    default:
        break;
    }
    return entity;
}

Entity CreateMob(MobType type)
{
    return Entity();
}
