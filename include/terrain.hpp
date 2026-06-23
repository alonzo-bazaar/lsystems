
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
    Terrain(std::vector<Vector3>&& inputVertex, int w, int h)
        : terrainVertex(std::move(inputVertex)), width(w), height(h) {
        genTerrainMesh();
    }

    static Terrain loadFromFile(const char* filename);

    // Ottimizzato: restituisce un riferimento costante, evitando copie inutili all'esterno
    [[nodiscard]] const std::vector<Vector3>& terrain_vertex() const {
        return terrainVertex;
    }

    [[nodiscard]] int height1() const {
        return height;
    }

    [[nodiscard]] int width1() const {
        return width;
    }

    void genTerrainMesh();

    void AssegnaShaderPBR(Shader pbrShader) {
        terrainModel.materials[0].shader = pbrShader;
    }

    void ImpostaTexturePBR(int mappaTipo, Texture2D texture) {
        terrainModel.materials[0].maps[mappaTipo].texture = texture;
    }

    void Disegna() {
        DrawModel(terrainModel, (Vector3){ 0.0f, 0.0f, 0.0f }, 1.0f, WHITE);
    }

    Model getModel() { return terrainModel; }

    // Aggiunto il distruttore per scaricare la memoria video quando il Terreno viene distrutto
    ~Terrain() {
        UnloadModel(terrainModel);
    }

protected:
    std::vector<Vector3> terrainVertex = {};
    int height;
    int width;
    Model terrainModel;
};

#endif //LSYSTEMS_TERRAIN_HLSYSTEMS_TERRAIN_HPP
