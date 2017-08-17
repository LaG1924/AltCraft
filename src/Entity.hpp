#pragma once

#include "Utility.hpp"
#include "Vector.hpp"

enum class ObjectType{
    Boat=1,
    ItemStack,
    AreaEffectCloud,
    Minecart=10,
    ActivatedTNT=50,
    EnderCrystal,
    TippedArrow=60,
    Snowball,
    Egg,
    FireBall,
    FireCharge,
    ThrownEnderpearl,
    WitherSkull,
    ShulkerBullet,
    LlamaSpit,
    FallingObjects=70,
    Itemframes,
    EyeOfEnder,
    ThrownPotion,
    ThrownExpBottle=75,
    FireworkRocket,
    LeashKnot,
    ArmorStand,
    EvocationFangs,
    FishingHook=90,
    SpectralArrow,
    DragonFireball=93,
};

enum MobType {
    Item = 1,
    XPOrb,
    AreaEffectCloud,
    ElderGuardian,
    WitherSkeleton,
    Stray,
    ThrownEgg,
    LeashKnot,
    Painting,
    Arrow,
    Snowball,
    Fireball,
    SmallFireball,
    ThrownEnderpearl,
    EyeOfEnderSignal,
    ThrownPotion,
    ThrownExpBottle,
    ItemFrame,
    WitherSkull,
    PrimedTnt,
    FallingSand,
    FireworksRocketEntity,
    Husk,
    SpectralArrow,
    ShulkerBullet,
    DragonFireball,
    ZombieVillager,
    SkeletonHorse,
    ZombieHorse,
    ArmorStand,
    Donkey,
    Mule,
    EvocationFangs,
    EvocationIllager,
    Vex,
    VindicationIllager,
    IllusionIllager,
    MinecartCommandBlock=40,
    Boat,
    MinecartRideable,
    MinecartChest,
    MinecartFurnace,
    MinecartTNT,
    MinecartHopper,
    MinecartSpawner,
    Creeper=50,
    Skeleton,
    Spider,
    Giant,
    Zombie,
    Slime,
    Ghast,
    PigZombie,
    Enderman,
    CaveSpider,
    Silverfish,
    Blaze,
    LavaSlime,
    EnderDragon,
    WitherBoss,
    Bat,
    Witch,
    Endermite,
    Guardian,
    Shulker,
    Pig=90,
    Sheep,
    Cow,
    Chicken,
    Squid,
    Wolf,
    MushroomCow,
    SnowMan,
    Ozelot,
    VillagerGolem,
    Horse,
    Rabbit,
    PolarBear,
    Llama,
    LlamaSpit,
    Parrot,
    Villager=120,
    EnderCrystal=200,
};

struct Entity {
    Uuid uuid;
    VectorF pos = 0;
    VectorF vel = 0;
    unsigned int entityId = 0;
    double yaw = 0;
    double pitch = 0;
    double width = 0.1;
    double height = 0.1;
    glm::vec3 renderColor;
    int entityType=0;
    bool isMob=false;
    static VectorF DecodeVelocity(short x, short y, short z);
    static VectorF DecodeDeltaPos(short deltaX, short deltaY, short deltaZ);
};

Entity CreateObject(ObjectType type);

Entity CreateMob(MobType type);