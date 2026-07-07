#pragma once

#ifndef LSYSTEMS_LIGHT_HPP_
#define LSYSTEMS_LIGHT_HPP_
#include "raylib.h"

typedef enum {
    LIGHT_DIRECTIONAL = 0,
    LIGHT_POINT,
    LIGHT_SPOT
} LightType;

// Light data
typedef struct {
    int type;
    int enabled;
    Vector3 position;
    Vector3 target;
    float color[4];
    float intensity;

    // Shader light parameters locations
    int typeLoc;
    int enabledLoc;
    int positionLoc;
    int targetLoc;
    int colorLoc;
    int intensityLoc;
} Light;

// Create a light and get shader locations
Light CreateLight(int type, Vector3 position, Vector3 target, Color color, float intensity, Shader shader);

// Update light properties on shader
// NOTE: Light shader locations should be available
void UpdateLight(Shader shader, Light light);

#endif //LSYSTEMS_LIGHT_HPP_
