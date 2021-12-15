#pragma once

struct PositionComponent {
    float x;
    float y;
};

struct HitboxComponent {
    float x;      // Rectangle top-left corner position x
    float y;      // Rectangle top-left corner position y
    float width;  // Rectangle width
    float height; // Rectangle height
};

struct PlayerComponent {
    float verticalSpeed;
    bool canJump;
};
