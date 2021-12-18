#pragma once

struct PhysicsComponent {
    float x;
    float y;

    float yVelocity;
    float xVelocity;

    bool isOnGround;
};

struct HitboxComponent {
    float x;      // Rectangle top-left corner position x
    float y;      // Rectangle top-left corner position y
    float width;  // Rectangle width
    float height; // Rectangle height
};

struct PlayerComponent {
    double lastJump = -1.0;
    double lastGrounded = -1.0;
};
