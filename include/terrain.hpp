#ifndef LSYSTEMS_TERRAIN_H
#define LSYSTEMS_TERRAIN_H
#include <cmath>
#include <iomanip>

#include "raylib.h"
#include <iostream>
#include <vector>

#include "raymath.h"
#include "rlgl.h"

class Terrain {
public:
    Terrain(std::vector<Vector3>&& input_vertex, int w, int h)
        : terrain_vertices(std::move(input_vertex)), width(w), height(h) {
        gen_terrain_mesh();
    }
    static Terrain load_from_file(const char* filename);
    void gen_terrain_mesh();
    void draw() {
        DrawModel(terrain_model, (Vector3){ 0.0f, 0.0f, 0.0f }, 1.0f, WHITE);
    }
    Model getModel() { return terrain_model; }
    ~Terrain() {
        UnloadModel(terrain_model);
    }

protected:
    std::vector<Vector3> terrain_vertices = {};
    int height;
    int width;
    Model terrain_model;
};

#endif // LSYSTEMS_TERRAIN_H
