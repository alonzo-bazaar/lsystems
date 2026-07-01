//
// Created by Notebook on 29/06/2026.
//

#ifndef LSYSTEMS_PLAYER_HPP
#define LSYSTEMS_PLAYER_HPP

#include "raylib.h"
#include "raymath.h"
#include "terrain.hpp"

struct Plane {
    Vector3 normal;
    float d;

    void normalize_plane() {
        const float length = sqrtf(normal.x * normal.x + normal.y * normal.y + normal.z * normal.z);
        if (length == 0.0f) return;
        normal.x /= length;
        normal.y /= length;
        normal.z /= length;
        d /= length;
    }
};

struct Body {
    Vector3 position;
    Vector3 velocity;
    Vector3 dir;
    bool is_grounded;
};

class Player {
public:
    static constexpr float GRAVITY = 32.0f;
    static constexpr float JUMP_FORCE = 12.0f;
    static constexpr float MAX_SPEED = 20.0f;
    static constexpr float CONTROL = 15.0f;
    // Grounded drag
    static constexpr float FRICTION = 0.86f;
    // Increasing air drag, increases strafing speed
    static constexpr float AIR_DRAG = 0.98f;
    static constexpr float MAX_ACCEL = 150.0f;
    static constexpr float CROUCH_SPEED = 5.0f;
    static constexpr float BOTTOM_HEIGHT = 0.5f;
    static constexpr float CROUCH_HEIGHT = 0.0f;
    static constexpr float STAND_HEIGHT = 1.0f;


    explicit Player(const Vector3 start_pos) {
        camera.position = start_pos;
        camera.target = {0.0f, 2.0f, 0.0f};
        camera.up = {0.0f, 1.0f, 0.0f};
        camera.fovy = 60.0f;
        camera.projection = CAMERA_PERSPECTIVE;
    }

    void update_shader_position(const Shader &shader) const;

    void update(int mode);

    void update_camera_first_person();

    void update_frustum();

    [[nodiscard]] bool can_see(const BoundingBox &box) const;

    void update_body(char side, char forward, bool is_jumping, bool is_crouching);

    [[nodiscard]] Camera get_camera() const { return camera; }

    void set_position(const Vector3 position) { camera.position = position; }

private:
    Camera camera = {};
    Plane frustum_planes[6] = {};
    Body body = {};
    Vector3 velocity = {};
    Vector3 dir = {};
    Vector2 look_rotation = {};
    Vector2 lean = {};
    Vector2 sensitivity = {0.001f, 0.001f};
    float head_timer = 0.0f;
    float walk_lerp = 0.0f;
    float head_lerp = 1.0f;
};

#endif //LSYSTEMS_PLAYER_HPP
