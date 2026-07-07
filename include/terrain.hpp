#pragma once
#ifndef LSYSTEMS_TERRAIN_HPP_
#define LSYSTEMS_TERRAIN_HPP_

#include <vector>
#include <map>
#include <cmath>

#include "raylib.h"
class Player;

class Terrain {
public:
    static constexpr int CHUNK_SIZE = 32;
    static constexpr int VERTEX_SIZE = CHUNK_SIZE + 1;
    static constexpr int RENDER_DISTANCE = 5;

    Terrain(std::vector<Vector3>&& inputVertex, const float wx, const float wz)
        : terrain_vertex(std::move(inputVertex)), world_x(wx), world_z(wz) {
    }

    static Terrain gen_perlin_chunk(float world_x, float world_z);

    void gen_chunk_mesh();

    static float perlin_fractal(float x, float z, int octaves, float persistence, float lacunarity);

    static void chunk_management(std::map<std::pair<int, int>, Terrain> &active_chunks, const Camera &camera, const Shader &shader, const Texture2D &mraTexture,
                                 const Texture2D &albedoTexture, const Texture2D &normalTexture);

    void draw_terrain() const;

    static void draw_visible_chunk(const std::map<std::pair<int, int>, Terrain> &active_chunks, const Player &player);

    static float get_height(float world_x, float world_z);

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
    BoundingBox terrain_bbox{};
};

#endif //LSYSTEMS_TERRAIN_HPP_
