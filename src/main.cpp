#include <raylib.h>
#include "tileson.hpp"
#include "MapLevel.hpp"

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
