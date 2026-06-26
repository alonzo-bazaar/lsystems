
#ifndef LSYSTEMS_TERRAIN_H
#define LSYSTEMS_TERRAIN_H
#include <cmath>

#include "raylib.h"
#include <iostream>
#include <vector>

#include "raymath.h"

class Terrain {
public:
    Terrain(std::vector<Vector3>&& inputVertex, const int l, const int w)
        : terrain_vertex(std::move(inputVertex)), length(l), width(w) {
    }

    static Terrain gen_perlin_chunk(int offset_x, int offset_z, int size);

    void gen_chunk_mesh();

    static float perlin_fractal(float x, float y, int octaves, float persistence, float lacunarity);

    void draw_terrain() const {
        DrawModel(terrain_model, (Vector3){ 0.0f, 0.0f, 0.0f }, 1.0f, WHITE);
    }

    [[nodiscard]] Model get_model() const { return terrain_model; }

protected:
    std::vector<Vector3> terrain_vertex = {};
    int length;
    int width;
    Model terrain_model{};
};

#endif //LSYSTEMS_TERRAIN_H
