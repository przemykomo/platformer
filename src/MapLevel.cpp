#include "MapLevel.hpp"
#include "box2d/b2_body.h"
#include "box2d/b2_fixture.h"
#include "box2d/b2_math.h"
#include "box2d/b2_polygon_shape.h"
#include "src/tileson.hpp"
#include <algorithm>
#include <raylib.h>

constexpr float BOX2D_SCALE = 1.0f / 16.0f;
constexpr float toBox2D(float px) { return px * BOX2D_SCALE; }
constexpr float fromBox2D(float m) { return m / BOX2D_SCALE; }

void MapLevel::collide(float x, float y, PhysicsComponent &physC,
                       HitboxComponent &hitbox) {
    tson::TileObject *tileObject = tileLayer->getTileObject(
        x / tsonMap->getTileSize().x, y / tsonMap->getTileSize().y);
    if (tileObject != nullptr) {
        tson::Vector2f tilePos = tileObject->getPosition();
        for (const tson::Object &object :
             tileObject->getTile()->getObjectgroup().getObjects()) {
            if (object.getObjectType() == tson::ObjectType::Rectangle) {
                tson::Vector2i bbSize = object.getSize();
                tson::Vector2i bbPos = object.getPosition();

                Rectangle collision =
                    GetCollisionRec({physC.x + hitbox.x, physC.y + hitbox.y,
                                     hitbox.width, hitbox.height},
                                    {tilePos.x + bbPos.x, tilePos.y + bbPos.y,
                                     static_cast<float>(bbSize.x),
                                     static_cast<float>(bbSize.y)});

                if (collision.width != collision.height &&
                    collision.width != 0 && collision.height != 0) {
                    if (collision.width < collision.height &&
                        physC.x < tilePos.x) {
                        physC.x -= collision.width;
                        physC.xVelocity = 0;
                    } else if (collision.width < collision.height &&
                               physC.x > tilePos.x) {
                        physC.x += collision.width;
                        physC.xVelocity = 0;
                    }

                    if (collision.width > collision.height &&
                        tilePos.y + 1 < physC.y + hitbox.y) {
                        physC.y += collision.height;
                        physC.yVelocity = 0.0f;
                    } else if (collision.width > collision.height &&
                               tilePos.y) {
                        physC.y -= collision.height;
                        physC.yVelocity = 0.0f;
                        physC.isOnGround = true;
                    }
                }
            }
        }
    }
}

MapLevel::MapLevel(tson::Tileson &tileson,
                   const std::filesystem::path &resources)
    : tsonMap(tileson.parse(resources / "level.json")),
      tileLayer(tsonMap->getLayer("Tile Layer 1")),
      objectLayer(tsonMap->getLayer("Object Layer 1")),
      colliderLayer(tsonMap->getLayer("collider layer")), world({0.0f, 10.0f}) {
    for (tson::Tileset &tileset : tsonMap->getTilesets()) {
        textures.emplace(&tileset,
                         LoadTexture((resources / tileset.getImage()).c_str()));
    }

    for (const tson::Object &collider : colliderLayer->getObjects()) {
        tson::Vector2i pos = collider.getPosition();
        tson::Vector2i size = collider.getSize();
        b2BodyDef groundBodyDef;
        groundBodyDef.position.Set(toBox2D(pos.x + (size.x / 2.0f)),
                                   toBox2D(pos.y + (size.y / 2.0f)));
        b2Body *groundBody = world.CreateBody(&groundBodyDef);
        b2PolygonShape groundBox;
        groundBox.SetAsBox(toBox2D(size.x / 2.0f), toBox2D(size.y / 2.0f));
        groundBody->CreateFixture(&groundBox, 0.0f);
    }

    entt::entity entity = registry.create();
    tson::Object *playerObject = objectLayer->firstObj("player");
    tson::Vector2i pos;

    if (playerObject != nullptr) {
        pos = playerObject->getPosition();
    }

    b2BodyDef playerBodyDef;
    playerBodyDef.type = b2_dynamicBody;
    playerBodyDef.fixedRotation = true;
    playerBodyDef.position.Set(toBox2D(pos.x), toBox2D(pos.y));
    // playerBodyDef.linearDamping = 0.04f;
    b2Body *playerBody = world.CreateBody(&playerBodyDef);
    b2PolygonShape dynamicBox;
    dynamicBox.SetAsBox(toBox2D(16.0f / 2.0f), toBox2D(16.0f / 2.0f));
    b2FixtureDef fixtureDef;
    fixtureDef.shape = &dynamicBox;
    fixtureDef.density = 1.0f;
    fixtureDef.friction = 0.0f;
    playerBody->CreateFixture(&fixtureDef);

    registry.emplace<PhysicsComponent>(entity, pos.x, pos.y, 0.0f, 0.0f, false,
                                       playerBody);
    registry.emplace<HitboxComponent>(entity, -8.0f, -16.0f, 16.0f, 16.0f);
    registry.emplace<PlayerComponent>(entity);

    camera.target = {100, 20};
    camera.offset = {GetScreenWidth() / 2.0f, GetScreenHeight() / 2.0f};
    camera.rotation = 0.0f;
    camera.zoom = 3.0f;
}

void MapLevel::frame() {
    world.Step(1.0f / 60.0f, 6, 2);
    registry.view<PhysicsComponent, HitboxComponent, PlayerComponent>().each(
        [this](PhysicsComponent &physC, HitboxComponent &hitbox,
               PlayerComponent &player) {
            {

                constexpr float MOVEMENT_FORCE = 15.0f;
                constexpr float MAX_VELOCITY = 8.0f;
                constexpr float STOP_FORCE = 5.0f;
                b2Vec2 velocity = physC.body->GetLinearVelocity();
                if (IsKeyDown(KEY_A) && !IsKeyDown(KEY_D)) {
                    if (velocity.x > -MAX_VELOCITY) {
                        physC.body->ApplyForce({-MOVEMENT_FORCE, 0.0f},
                                               physC.body->GetWorldCenter(),
                                               false);
                    }
                } else if (IsKeyDown(KEY_D) && !IsKeyDown(KEY_A)) {
                    if (velocity.x < MAX_VELOCITY) {
                        physC.body->ApplyForce({MOVEMENT_FORCE, 0.0f},
                                               physC.body->GetWorldCenter(),
                                               false);
                    }
                } else {
                    physC.body->ApplyForce({velocity.x * -STOP_FORCE, 0.0f},
                                           physC.body->GetWorldCenter(), false);
                }

                if (IsKeyPressed(KEY_SPACE)) {
                    physC.body->ApplyLinearImpulseToCenter({0.0f, -5.0f},
                                                           false);
                }
            }

            b2Vec2 pos = physC.body->GetPosition();
            camera.target = {fromBox2D(pos.x), fromBox2D(pos.y)};
        });
    camera.offset = {GetScreenWidth() / 2.0f, GetScreenHeight() / 2.0f};
    BeginMode2D(camera);

    for (auto &[pos, tileObject] : tileLayer->getTileObjects()) {
        tson::Rect drawingRect = tileObject.getDrawingRect();
        tson::Vector2f position = tileObject.getPosition();
        const Texture2D &texture = textures[tileObject.getTile()->getTileset()];

        DrawTextureQuad(
            texture,
            {static_cast<float>(drawingRect.width) / texture.width,
             static_cast<float>(drawingRect.height) / texture.height},
            {static_cast<float>(drawingRect.x) / texture.width,
             static_cast<float>(drawingRect.y) / texture.height},
            {position.x, position.y, static_cast<float>(drawingRect.width),
             static_cast<float>(drawingRect.height)},
            WHITE);
    }

    auto drawingView =
        registry.view<const PhysicsComponent, const HitboxComponent>();
    drawingView.each(
        [](const PhysicsComponent &position, const HitboxComponent &hitbox) {
            const b2Vec2 &pos = position.body->GetPosition();
            DrawRectangleRec({fromBox2D(pos.x) - (hitbox.width / 2.0f),
                              fromBox2D(pos.y) - (hitbox.height / 2.0f),
                              hitbox.width, hitbox.height},
                             RED);
        });

    for (const tson::Object &collider : colliderLayer->getObjects()) {
        tson::Vector2i pos = collider.getPosition();
        tson::Vector2i size = collider.getSize();
        DrawRectangleLines(pos.x, pos.y, size.x, size.y, WHITE);
    }

    EndMode2D();
}

MapLevel::~MapLevel() {
    for (auto &[tileset, texture] : textures) {
        UnloadTexture(texture);
    }
}
