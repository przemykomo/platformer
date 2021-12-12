#include "raylib.h"
#include "tileson.hpp"
#include <filesystem>
#include <iostream>
#include <memory>

constexpr float scale = 2.0f;

class Map {
  public:
    std::unique_ptr<tson::Map> tsonMap;
    tson::Layer *tileLayer;
    std::map<tson::Tileset *, Texture2D> textures;

    Map(tson::Tileson &tileson, const std::filesystem::path &resources)
        : tsonMap(tileson.parse(resources / "level.json")),
          tileLayer(tsonMap->getLayer("Tile Layer 1")) {
        for (tson::Tileset &tileset : tsonMap->getTilesets()) {
            textures.emplace(
                &tileset,
                LoadTexture((resources / tileset.getImage()).c_str()));
        }
    }

    void frame() {
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
                {position.x * scale, position.y * scale,
                 drawingRect.width * scale, drawingRect.height * scale},
                WHITE);

            for (const tson::Object &object : tileObject.getTile()->getObjectgroup().getObjects()) {
                if (object.getObjectType() == tson::ObjectType::Rectangle) {
                    tson::Vector2i bbSize = object.getSize();
                    tson::Vector2i bbPos = object.getPosition();

                    DrawRectangleLines((position.x + bbPos.x) * scale, (position.y + bbPos.y) * scale, bbSize.x * scale, bbSize.y * scale, RED);
                }
            }
        }
    }

    ~Map() {
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
        Map map(tileson, "./res");

        while (!WindowShouldClose()) {
            BeginDrawing();

            ClearBackground(GRAY);

            DrawText("nub", 190, 200, 20, YELLOW);

            map.frame();

            EndDrawing();
        }
    }

    CloseWindow();

    return 0;
}
