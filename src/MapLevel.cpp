#include "MapLevel.hpp"

void MapLevel::collide(float x, float y, PositionComponent &position,
                       HitboxComponent &hitbox, PlayerComponent &player) {
    tson::TileObject *tileObject = tileLayer->getTileObject(
        x / tsonMap->getTileSize().x, y / tsonMap->getTileSize().y);
    if (tileObject != nullptr) {
        tson::Vector2f tilePos = tileObject->getPosition();
        for (const tson::Object &object :
             tileObject->getTile()->getObjectgroup().getObjects()) {
            if (object.getObjectType() == tson::ObjectType::Rectangle) {
                tson::Vector2i bbSize = object.getSize();
                tson::Vector2i bbPos = object.getPosition();

                Rectangle collision = GetCollisionRec(
                    {position.x + hitbox.x, position.y + hitbox.y, hitbox.width,
                     hitbox.height},
                    {tilePos.x + bbPos.x, tilePos.y + bbPos.y,
                     static_cast<float>(bbSize.x),
                     static_cast<float>(bbSize.y)});

                if (collision.width != collision.height &&
                    collision.width != 0 && collision.height != 0) {
                    if (collision.width < collision.height &&
                        position.x < tilePos.x) {
                        position.x -= collision.width;
                    } else if (collision.width < collision.height &&
                               position.x > tilePos.x) {
                        position.x += collision.width;
                    }

                    if (collision.width > collision.height &&
                        tilePos.y + 1 < position.y + hitbox.y) {
                        position.y += collision.height;
                        player.verticalSpeed = 0.0f;
                    } else if (collision.width > collision.height &&
                               tilePos.y) {
                        position.y -= collision.height;
                        player.canJump = true;
                        player.verticalSpeed = 0.0f;
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
    registry.emplace<PositionComponent>(entity, 100.0f, 20.0f);
    registry.emplace<HitboxComponent>(entity, -8.0f, -16.0f, 16.0f, 16.0f);
    registry.emplace<PlayerComponent>(entity, 0.0f, false);

    camera.target = {100, 20};
    camera.offset = {GetScreenWidth() / 2.0f, GetScreenHeight() / 2.0f};
    camera.rotation = 0.0f;
    camera.zoom = 3.0f;
}

void MapLevel::frame() {
    registry.view<PositionComponent, HitboxComponent, PlayerComponent>().each(
        [this](PositionComponent &position, HitboxComponent &hitbox,
               PlayerComponent &player) {
            float delta = GetFrameTime();
            if (IsKeyDown(KEY_LEFT)) {
                position.x -= 200.0f * delta;
            }

            if (IsKeyDown(KEY_RIGHT)) {
                position.x += 200.0f * delta;
            }

            if (IsKeyDown(KEY_SPACE) && player.canJump) {
                player.verticalSpeed = -80.0f;
                player.canJump = false;
            }

            player.verticalSpeed += 80.0f * delta;
            position.y += player.verticalSpeed * delta;

            if (!IsKeyDown(KEY_E)) {
                collide(position.x + hitbox.x, position.y + hitbox.y, position,
                        hitbox, player);
                collide(position.x + hitbox.x,
                        position.y + hitbox.y + hitbox.height, position, hitbox,
                        player);
                collide(position.x + hitbox.x + hitbox.width,
                        position.y + hitbox.y, position, hitbox, player);
                collide(position.x + hitbox.x + hitbox.width,
                        position.y + hitbox.y + hitbox.height, position, hitbox,
                        player);
            }

            camera.target = {position.x, position.y};
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
        registry.view<const PositionComponent, const HitboxComponent>();
    drawingView.each(
        [](const PositionComponent &position, const HitboxComponent &hitbox) {
            DrawRectangle(position.x + hitbox.x, position.y + hitbox.y,
                          hitbox.width, hitbox.height, RED);
        });

    EndMode2D();
}

MapLevel::~MapLevel() {
    for (auto &[tileset, texture] : textures) {
        UnloadTexture(texture);
    }
}
