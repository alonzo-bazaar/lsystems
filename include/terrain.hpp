
#ifndef LSYSTEMS_TERRAIN_H
#define LSYSTEMS_TERRAIN_H
#include <cmath>

#include "raylib.h"
#include <iostream>
#include <vector>
#include <map>

#include "raymath.h"

class Terrain {
public:
    static constexpr int CHUNK_SIZE = 32;
    static constexpr int RENDER_DISTANCE = 4;

    Terrain(std::vector<Vector3>&& inputVertex, const float wx, const float wz)
        : terrain_vertex(std::move(inputVertex)), world_x(wx), world_z(wz) {
    }

    static Terrain gen_perlin_chunk(float world_x, float world_z);

    void gen_chunk_mesh();

    static float perlin_fractal(float x, float z, int octaves, float persistence, float lacunarity);

    static void chunk_management(std::map<std::pair<int, int>, Terrain> &active_chunks, const Camera &camera, const Shader &shader, const Texture2D &mraTexture,
                                 const Texture2D &albedoTexture, const Texture2D &normalTexture);

    void draw_terrain() const {
        DrawModel(terrain_model, (Vector3){world_x, 0.0f, world_z}, 1.0f, WHITE);
    }

    [[nodiscard]] Model get_model() const { return terrain_model; }

    [[nodiscard]] float get_world_x() const {
        return world_x;
    }

    [[nodiscard]] float get_world_z() const {
        return world_z;
    }

protected:
    std::vector<Vector3> terrain_vertex = {};
    float world_x, world_z;
    Model terrain_model{};
};

#endif //LSYSTEMS_TERRAIN_H
