#pragma once
#include <raylib.h>
#include <entt/entt.hpp>
#include "box2d/b2_body.h"
#include "box2d/b2_world.h"
#include "tileson.hpp"
#include "components.hpp"
#include <box2d/box2d.h>

struct MapLevel {

  public:
    std::unique_ptr<tson::Map> tsonMap;
    tson::Layer *tileLayer;
    tson::Layer *objectLayer;
    tson::Layer *colliderLayer;

    std::map<tson::Tileset *, Texture2D> textures;
    Camera2D camera;
    entt::registry registry;

    b2World world;
    b2Body *playerBody;

    void collide(float x, float y, PhysicsComponent &position,
                 HitboxComponent &hitbox);
    void frame();

    MapLevel(tson::Tileson &tileson, const std::filesystem::path &resources);
    ~MapLevel();
};
