#include "raylib.h"
#include "tileson.hpp"
#include <filesystem>
#include <iostream>
#include <memory>

struct Player {
    Vector2 position;
    float verticalSpeed;
    bool canJump;
    Rectangle hitbox;
};

class MapLevel {
  public:
    std::unique_ptr<tson::Map> tsonMap;
    tson::Layer *tileLayer;
    std::map<tson::Tileset *, Texture2D> textures;
    Player player;
    Camera2D camera;

    void collide(float x, float y) {
        tson::TileObject *tileObject = tileLayer->getTileObject(
            x / tsonMap->getTileSize().x, y / tsonMap->getTileSize().y);
        if (tileObject != nullptr) {
            tson::Vector2f tilePos = tileObject->getPosition();
            for (const tson::Object &object :
                 tileObject->getTile()->getObjectgroup().getObjects()) {
                if (object.getObjectType() == tson::ObjectType::Rectangle) {
                    tson::Vector2i bbSize = object.getSize();
                    tson::Vector2i bbPos = object.getPosition();

                    DrawRectangleLines((tilePos.x + bbPos.x),
                                       (tilePos.y + bbPos.y), bbSize.x,
                                       bbSize.y, RED);

                    Rectangle collision = GetCollisionRec(
                        {player.position.x + player.hitbox.x,
                         player.position.y + player.hitbox.y,
                         player.hitbox.width, player.hitbox.height},
                        {tilePos.x + bbPos.x, tilePos.y + bbPos.y,
                         static_cast<float>(bbSize.x),
                         static_cast<float>(bbSize.y)});

                    if (collision.width != collision.height &&
                        collision.width != 0 && collision.height != 0) {
                        if (collision.width < collision.height &&
                            player.position.x < tilePos.x) {
                            player.position.x -= collision.width;
                        } else if (collision.width < collision.height &&
                                   player.position.x > tilePos.x) {
                            player.position.x += collision.width;
                        }

                        if (collision.width > collision.height &&
                            tilePos.y + 1 < player.position.y + player.hitbox.y) {
                            player.position.y += collision.height;
                            player.verticalSpeed = 0.0f;
                        } else if (collision.width > collision.height &&
                                   tilePos.y) {
                            player.position.y -= collision.height;
                            player.canJump = true;
                            player.verticalSpeed = 0.0f;
                        }
                    }
                }
            }
        }
    }

    MapLevel(tson::Tileson &tileson, const std::filesystem::path &resources)
        : tsonMap(tileson.parse(resources / "level.json")),
          tileLayer(tsonMap->getLayer("Tile Layer 1")) {
        for (tson::Tileset &tileset : tsonMap->getTilesets()) {
            textures.emplace(
                &tileset,
                LoadTexture((resources / tileset.getImage()).c_str()));
        }

        player.position = {100, 20};
        player.verticalSpeed = 0;
        player.canJump = false;
        player.hitbox = {-8, -16, 16, 16};

        camera.target = player.position;
        camera.offset = {GetScreenWidth() / 2.0f, GetScreenHeight() / 2.0f};
        camera.rotation = 0.0f;
        camera.zoom = 3.0f;
    }

    void frame() {
        float delta = GetFrameTime();

        if (IsKeyDown(KEY_LEFT)) {
            player.position.x -= 200.0f * delta;
        }

        if (IsKeyDown(KEY_RIGHT)) {
            player.position.x += 200.0f * delta;
        }

        if (IsKeyDown(KEY_UP)) {
            player.position.y -= 200.0f * delta;
        }

        if (IsKeyDown(KEY_SPACE) && player.canJump) {
            player.verticalSpeed = -80.0f;
            player.canJump = false;
        }

        player.verticalSpeed += 80.0f * delta;
        player.position.y += player.verticalSpeed * delta;

        camera.target = player.position;
        camera.offset = {GetScreenWidth() / 2.0f, GetScreenHeight() / 2.0f};
        BeginMode2D(camera);

        for (auto &[pos, tileObject] : tileLayer->getTileObjects()) {
            tson::Rect drawingRect = tileObject.getDrawingRect();
            tson::Vector2f position = tileObject.getPosition();
            const Texture2D &texture =
                textures[tileObject.getTile()->getTileset()];

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

        DrawRectangleRec({player.position.x + player.hitbox.x,
                          player.position.y + player.hitbox.y,
                          player.hitbox.width, player.hitbox.height},
                         RED);

        if (!IsKeyDown(KEY_E)) {
            collide(player.position.x + player.hitbox.x,
                    player.position.y + player.hitbox.y);
            collide(player.position.x + player.hitbox.x,
                    player.position.y + player.hitbox.y + player.hitbox.height);
            collide(player.position.x + player.hitbox.x + player.hitbox.width,
                    player.position.y + player.hitbox.y);
            collide(player.position.x + player.hitbox.x + player.hitbox.width,
                    player.position.y + player.hitbox.y + player.hitbox.height);
        }
        EndMode2D();
    }

    ~MapLevel() {
        for (auto &[tileset, texture] : textures) {
            UnloadTexture(texture);
        }
    }
};

int main(int argc, const char **argv) {
    InitWindow(800, 450, "DevWindow");

    SetTargetFPS(60);

    {
        tson::Tileson tileson;
        MapLevel map(tileson, "./res");

        while (!WindowShouldClose()) {
            BeginDrawing();
            ClearBackground(GRAY);
            map.frame();
            EndDrawing();
        }
    }

    CloseWindow();

    return 0;
}
