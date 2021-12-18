#pragma once
#include <raylib.h>
#include <entt/entt.hpp>
#include "tileson.hpp"
#include "components.hpp"

struct MapLevel {

  public:
    std::unique_ptr<tson::Map> tsonMap;
    tson::Layer *tileLayer;
    std::map<tson::Tileset *, Texture2D> textures;
    Camera2D camera;
    entt::registry registry;

    void collide(float x, float y, PhysicsComponent &position,
                 HitboxComponent &hitbox);
    void frame();

    MapLevel(tson::Tileson &tileson, const std::filesystem::path &resources);
    ~MapLevel();
};
