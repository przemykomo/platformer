#include "MapLevel.hpp"
#include <raylib.h>
#include <algorithm>

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
      tileLayer(tsonMap->getLayer("Tile Layer 1")) {
    for (tson::Tileset &tileset : tsonMap->getTilesets()) {
        textures.emplace(&tileset,
                         LoadTexture((resources / tileset.getImage()).c_str()));
    }

    entt::entity entity = registry.create();
    registry.emplace<PhysicsComponent>(entity, 100.0f, 20.0f, 0.0f, 0.0f,
                                       false);
    registry.emplace<HitboxComponent>(entity, -8.0f, -16.0f, 16.0f, 16.0f);
    registry.emplace<PlayerComponent>(entity);

    camera.target = {100, 20};
    camera.offset = {GetScreenWidth() / 2.0f, GetScreenHeight() / 2.0f};
    camera.rotation = 0.0f;
    camera.zoom = 3.0f;
}

void MapLevel::frame() {
    registry.view<PhysicsComponent, HitboxComponent, PlayerComponent>().each(
        [this](PhysicsComponent &physC, HitboxComponent &hitbox,
               PlayerComponent &player) {
            {
                double now = GetTime();
                if (IsKeyPressed(KEY_SPACE)) {
                    player.lastJump = GetTime();
                }

                bool wantsToJump = false;

                if (player.lastJump >= 0 && now - player.lastJump < 0.1) {
                    wantsToJump = true;
                }

                if (physC.isOnGround) {
                    player.lastGrounded = now;
                }

                if (wantsToJump && now - player.lastGrounded < 0.1) {
                    physC.yVelocity = -130.0f;
                }

                constexpr float minimumJumpVelocity = -60.0f;
                if (IsKeyUp(KEY_SPACE) &&
                    (physC.yVelocity < minimumJumpVelocity ||
                     physC.yVelocity == -130.0f)) {
                    physC.yVelocity = minimumJumpVelocity;
                }
            }
            {
                float delta = GetFrameTime();
                constexpr float movementAcceleration = 900.0f;
                constexpr float maxMovementSpeed = 100.0f;
                constexpr float gravity = 200.0f;
                if (IsKeyDown(KEY_A)) {
                    physC.xVelocity -= movementAcceleration * delta;
                }

                if (IsKeyDown(KEY_D)) {
                    physC.xVelocity += movementAcceleration * delta;
                }

                constexpr float damping = 4.0f;
                physC.xVelocity /= 1 + damping * delta;
                physC.xVelocity = std::clamp(physC.xVelocity, -maxMovementSpeed, maxMovementSpeed);
                physC.yVelocity += gravity * delta;
                physC.y += physC.yVelocity * delta;
                physC.x += physC.xVelocity * delta;
            }
            if (!IsKeyDown(KEY_E)) {
                physC.isOnGround = false;

                collide(physC.x + hitbox.x, physC.y + hitbox.y, physC, hitbox);
                collide(physC.x + hitbox.x, physC.y + hitbox.y + hitbox.height,
                        physC, hitbox);
                collide(physC.x + hitbox.x + hitbox.width, physC.y + hitbox.y,
                        physC, hitbox);
                collide(physC.x + hitbox.x + hitbox.width,
                        physC.y + hitbox.y + hitbox.height, physC, hitbox);
            }

            camera.target = {physC.x, physC.y};
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
            DrawRectangleRec({position.x + hitbox.x, position.y + hitbox.y,
                              hitbox.width, hitbox.height},
                             RED);
        });

    EndMode2D();
}

MapLevel::~MapLevel() {
    for (auto &[tileset, texture] : textures) {
        UnloadTexture(texture);
    }
}
