//
// Created by Notebook on 29/06/2026.
//

#include "player.hpp"

// La logica della funzione la spostiamo qui
void Player::update_shader_position(const Shader &shader) const {
    const float cameraPos[3] = {camera.position.x, camera.position.y, camera.position.z};
    SetShaderValue(
        shader,
        shader.locs[SHADER_LOC_VECTOR_VIEW],
        cameraPos,
        SHADER_UNIFORM_VEC3
        );
}

void Player::update(const int mode) { UpdateCamera(&camera, mode); }

// Update body considering current world state
void Player::update_body(const char side, const char forward, const bool is_jumping, const bool is_crouching) {
    Vector2 input = {static_cast<float>(side), static_cast<float>(-forward)};

#if defined(NORMALIZE_INPUT)
    // Slow down diagonal movement
    if ((side != 0) && (forward != 0)) input = Vector2Normalize(input);
#endif

    const float delta = GetFrameTime();

    if (!body.is_grounded) body.velocity.y -= GRAVITY * delta;

    if (body.is_grounded && is_jumping) {
        body.velocity.y = JUMP_FORCE;
        body.is_grounded = false;
    }

    const Vector3 front = {sinf(look_rotation.x), 0.0f, cosf(look_rotation.x)};
    const Vector3 right = {cosf(-look_rotation.x), 0.0, sinf(-look_rotation.x)};

    const Vector3 desired_dir = {
        input.x * right.x + input.y * front.x,
        0.0f, input.x * right.z + input.y * front.z,
    };

    body.dir = Vector3Lerp(body.dir, desired_dir, CONTROL * delta);

    const float decel = (body.is_grounded ? FRICTION : AIR_DRAG);
    Vector3 horizontal_velocity = {body.velocity.x * decel, 0.0f, body.velocity.z * decel};

    float horizontal_velocity_magnitude = Vector3Length(horizontal_velocity); // Magnitude
    if (horizontal_velocity_magnitude < MAX_SPEED * 0.01f) horizontal_velocity = {};

    // This is what creates strafing
    const float speed = Vector3DotProduct(horizontal_velocity, body.dir);

    // Whenever the amount of acceleration to add is clamped by the maximum acceleration constant,
    // a Player can make the speed faster by bringing the direction closer to horizontal velocity angle
    // More info here: https://youtu.be/v3zT3Z5apaM?t=165
    const float max_speed = is_crouching ? CROUCH_SPEED : MAX_SPEED;
    const float accel = Clamp(max_speed - speed, 0.f, MAX_ACCEL * delta);

    horizontal_velocity.x += body.dir.x * accel;
    horizontal_velocity.z += body.dir.z * accel;
    body.velocity.x = horizontal_velocity.x;
    body.velocity.z = horizontal_velocity.z;

    body.position.x += body.velocity.x * delta;
    body.position.y += body.velocity.y * delta;
    body.position.z += body.velocity.z * delta;

    // Fancy collision system against the floor
    const float terrain_height = Terrain::get_height(body.position.x, body.position.z);
    if (body.position.y <= terrain_height) {
        body.position.y = terrain_height;
        body.velocity.y = 0.0f;
        body.is_grounded = true; // Enable jumping
    } else body.is_grounded = false;

    head_lerp = Lerp(head_lerp, is_crouching ? CROUCH_HEIGHT : STAND_HEIGHT, 20.0f * delta);
    camera.position = (Vector3){
        body.position.x,
        body.position.y + (BOTTOM_HEIGHT + head_lerp),
        body.position.z,
    };

    if (body.is_grounded && (forward != 0 || side != 0)) {
        head_timer += delta * 3.0f;
        walk_lerp = Lerp(walk_lerp, 1.0f, 10.0f * delta);
        camera.fovy = Lerp(camera.fovy, 55.0f, 5.0f * delta);
    } else {
        walk_lerp = Lerp(walk_lerp, 0.0f, 10.0f * delta);
        camera.fovy = Lerp(camera.fovy, 60.0f, 5.0f * delta);
    }

    lean.x = Lerp(lean.x, side * 0.02f, 10.0f * delta);
    lean.y = Lerp(lean.y, forward * 0.015f, 10.0f * delta);
}

// Update camera for FPS behavior
void Player::update_camera_first_person() {
    constexpr Vector3 up = {0.0f, 1.0f, 0.0f};
    constexpr Vector3 target_offset = {0.0f, 0.0f, -1.0f};
    auto [x, y] = GetMouseDelta();
    look_rotation.x -= x * sensitivity.x;
    look_rotation.y += y * sensitivity.y;

    // Left and right
    const Vector3 yaw = Vector3RotateByAxisAngle(target_offset, up, look_rotation.x);

    // Clamp view up
    float max_angle_up = Vector3Angle(up, yaw);
    max_angle_up -= 0.001f; // Avoid numerical errors
    if (-(look_rotation.y) > max_angle_up) { look_rotation.y = -max_angle_up; }

    // Clamp view down
    float max_angle_down = Vector3Angle(Vector3Negate(up), yaw);
    max_angle_down *= -1.0f; // Downwards angle is negative
    max_angle_down += 0.001f; // Avoid numerical errors
    if (-(look_rotation.y) < max_angle_down) { look_rotation.y = -max_angle_down; }

    // Up and down
    const Vector3 right = Vector3Normalize(Vector3CrossProduct(yaw, up));

    // Rotate view vector around right axis
    const float pitch_angle = Clamp(
        -look_rotation.y - lean.y,
        -PI / 2 + 0.0001f,
        PI / 2 - 0.0001f
    );
    // Clamp angle so it doesn't go past straight up or straight down
    const Vector3 pitch = Vector3RotateByAxisAngle(yaw, right, pitch_angle);

    // Head animation
    // Rotate up direction around forward axis
    const float head_sin = sinf(head_timer * PI);
    const float head_cos = cosf(head_timer * PI);
    constexpr float step_rotation = 0.01f;
    camera.up = Vector3RotateByAxisAngle(up, pitch, head_sin * step_rotation + lean.x);

    // Camera BOB
    constexpr float bob_side = 0.1f;
    constexpr float bob_up = 0.15f;
    Vector3 bobbing = Vector3Scale(right, head_sin * bob_side);
    bobbing.y = fabsf(head_cos * bob_up);

    camera.position = Vector3Add(camera.position, Vector3Scale(bobbing, walk_lerp));
    camera.target = Vector3Add(camera.position, pitch);
}
