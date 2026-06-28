//
// Created by Notebook on 22/06/2026.
//
#include <map>
#include <terrain.hpp>

// Genera un chunk con coordinate che partono da offset con grandezza size usando perlin
Terrain Terrain::gen_perlin_chunk(const float world_x, const float world_z) {
    std::vector<Vector3> vertices;
    int vertex_size = CHUNK_SIZE + 1;
    vertices.resize(vertex_size * vertex_size);

    float z = world_z;
    for (int r = 0; r < vertex_size; r++, z++) {
        float x = world_x;
        for (int c = 0; c < vertex_size; c++, x++) {
            float amplitude = 100.0f;
            float scale = 0.006f;
            int index = r * vertex_size + c;

            vertices[index].x = static_cast<float>(c);
            vertices[index].z = static_cast<float>(r);

            float val = perlin_fractal(x * scale, z * scale, 8, 0.5f, 2.0f);
            vertices[index].y = val * amplitude;
        }
    }
    Terrain chunk(std::move(vertices), world_x, world_z);
    chunk.gen_chunk_mesh();
    return chunk;
}


void Terrain::gen_chunk_mesh() {
    Mesh mesh = {};
    int num_squares = CHUNK_SIZE * CHUNK_SIZE;
    mesh.vertexCount = (CHUNK_SIZE + 1) * (CHUNK_SIZE + 1);
    mesh.triangleCount = num_squares * 2;

    // Allocazione buffer
    mesh.vertices = static_cast<float *>(MemAlloc(mesh.vertexCount * 3 * sizeof(float)));
    mesh.texcoords = static_cast<float *>(MemAlloc(mesh.vertexCount * 2 * sizeof(float)));
    mesh.normals = static_cast<float *>(MemAlloc(mesh.vertexCount * 3 * sizeof(float)));
    mesh.indices = static_cast<unsigned short *>(MemAlloc(mesh.triangleCount * 3 * sizeof(unsigned short)));

    float inv_width_minus_one = 1.0f / static_cast<float>(CHUNK_SIZE);

    // Assegnazione valori a mesh
    for (int r = 0; r < CHUNK_SIZE + 1; r++) {
        for (int c = 0; c < CHUNK_SIZE + 1; c++) {
            int i = r * (CHUNK_SIZE + 1) + c;

            // Vertici
            mesh.vertices[i * 3 + 0] = terrain_vertex[i].x;
            mesh.vertices[i * 3 + 1] = terrain_vertex[i].y;
            mesh.vertices[i * 3 + 2] = terrain_vertex[i].z;

            // UV
            mesh.texcoords[i * 2 + 0] = static_cast<float>(c) * inv_width_minus_one;
            mesh.texcoords[i * 2 + 1] = static_cast<float>(r) * inv_width_minus_one;

            // Calcolo normali
            Vector3 current = terrain_vertex[i];
            Vector3 left = (c > 0) ? terrain_vertex[i - 1] : current;
            Vector3 right = (c < CHUNK_SIZE) ? terrain_vertex[i + 1] : current;
            Vector3 top = (r > 0) ? terrain_vertex[i - (CHUNK_SIZE + 1)] : current;
            Vector3 bottom = (r < CHUNK_SIZE) ? terrain_vertex[i + CHUNK_SIZE + 1] : current;

            Vector3 tangent = {right.x - left.x, right.y - left.y, right.z - left.z};
            Vector3 bitangent = {bottom.x - top.x, bottom.y - top.y, bottom.z - top.z};

            // Prodotto vettoriale
            Vector3 normal = {
                (bitangent.y * tangent.z) - (bitangent.z * tangent.y),
                (bitangent.z * tangent.x) - (bitangent.x * tangent.z),
                (bitangent.x * tangent.y) - (bitangent.y * tangent.x)
            };

            // Normalizzazione
            float l = sqrtf(normal.x * normal.x + normal.y * normal.y + normal.z * normal.z);
            mesh.normals[i * 3 + 0] = normal.x / l;
            mesh.normals[i * 3 + 1] = normal.y / l;
            mesh.normals[i * 3 + 2] = normal.z / l;
        }
    }

    // Caricamento indici triangoli
    int index_count = 0;
    for (int r = 0; r < CHUNK_SIZE; r++) {
        for (int c = 0; c < CHUNK_SIZE; c++) {
            int top_left = r * (CHUNK_SIZE + 1) + c;
            int top_right = top_left + 1;
            int bottom_left = (r + 1) * (CHUNK_SIZE + 1) + c;
            int bottom_right = bottom_left + 1;

            mesh.indices[index_count++] = top_left;
            mesh.indices[index_count++] = bottom_left;
            mesh.indices[index_count++] = top_right;

            mesh.indices[index_count++] = top_right;
            mesh.indices[index_count++] = bottom_left;
            mesh.indices[index_count++] = bottom_right;
        }
    }

    GenMeshTangents(&mesh);
    UploadMesh(&mesh, false);
    terrain_model = LoadModelFromMesh(mesh);
}


Vector2 random_gradient(int ix, int iy) {
    // Nessun gradiente pre-calcolato quindi funziona per qualsiasi coordinata
    const unsigned w = 8 * sizeof(unsigned);
    const unsigned s = w / 2;
    unsigned a = static_cast<unsigned>(ix) * 3284157443u;
    unsigned b = static_cast<unsigned>(iy) * 1911520717u;

    a *= 3284157443;

    b ^= a << s | a >> (w - s);
    b *= 1911520717;

    a ^= b << s | b >> (w - s);
    a *= 2048419325;
    auto random = static_cast<float>(a * (3.14159265 / ~(~0u >> 1))); // in [0, 2*Pi]

    // Crea vettore dall'angolo
    Vector2 v;
    v.x = sin(random);
    v.y = cos(random);

    return v;
}


// Calcola prodotto scalare tra distanza e gradiente
float dot_grid_gradient(int ix, int iy, float x, float y) {
    // Calcola gradiente dalle coordinate
    Vector2 gradient = random_gradient(ix, iy);

    // Calcola il vettore di distanza
    float dx = x - static_cast<float>(ix);
    float dy = y - static_cast<float>(iy);

    // Calcola il prodotto scalare
    return dx * gradient.x + dy * gradient.y;
}

float interpolate(float a0, float a1, float w) {
    return static_cast<float>((a1 - a0) * (3.0 - w * 2.0) * w * w + a0);
}


// Campiona rumore alle coordinate x,y
float perlin(float x, float y) {
    // Determina coordinate dell'angolo della griglia
    int x0 = static_cast<int>(floor(x));
    int y0 = static_cast<int>(floor(y));
    int x1 = x0 + 1;
    int y1 = y0 + 1;

    // Calcolo pesi per interpolazione
    float sx = x - static_cast<float>(x0);
    float sy = y - static_cast<float>(y0);

    // Interpola angoli in alto
    float n0 = dot_grid_gradient(x0, y0, x, y);
    float n1 = dot_grid_gradient(x1, y0, x, y);
    float ix0 = interpolate(n0, n1, sx);

    // Calcola e interpola angoli in basso
    n0 = dot_grid_gradient(x0, y1, x, y);
    n1 = dot_grid_gradient(x1, y1, x, y);
    float ix1 = interpolate(n0, n1, sx);

    // Interpola i valori precedenti in y
    float value = interpolate(ix0, ix1, sy);

    return value;
}


// Genera frattale di perlin
float Terrain::perlin_fractal(float x, float z, int octaves, float persistence, float lacunarity) {
    float total = 0.0f;
    float frequency = 1.0f;
    float amplitude = 1.0f;
    float max_value = 0.0f; // Usato per normalizzare il risultato finale

    for (int i = 0; i < octaves; i++) {
        total += perlin(x * frequency, z * frequency) * amplitude;

        max_value += amplitude;

        amplitude *= persistence; // Diminuisce l'impatto dei dettagli piccoli
        frequency *= lacunarity; // Aumenta la frequenza (il dettaglio)
    }

    return total / max_value; // Restituisce un valore tra -1 e 1
}

// Gestione chunk nel raggio di RENDER_DISTANCE
void Terrain::chunk_management(std::map<std::pair<int, int>, Terrain> &active_chunks, const Camera &camera,
                               const Shader &shader, const Texture2D &mraTexture,
                               const Texture2D &albedoTexture, const Texture2D &normalTexture) {
    const int cam_chunk_x = static_cast<int>(floorf(camera.position.x / CHUNK_SIZE));
    const int cam_chunk_z = static_cast<int>(floorf(camera.position.z / CHUNK_SIZE));
    // Creazione chunk entro RENDER_DISTANCE
    for (int z = -RENDER_DISTANCE; z <= RENDER_DISTANCE; z++) {
        for (int x = -RENDER_DISTANCE; x <= RENDER_DISTANCE; x++) {
            std::pair chunk_pos = {cam_chunk_x + x, cam_chunk_z + z};
            if (active_chunks.find(chunk_pos) == active_chunks.end()) {
                float world_z = static_cast<float>(cam_chunk_z + z) * (CHUNK_SIZE);
                float world_x = static_cast<float>(cam_chunk_x + x) * (CHUNK_SIZE);
                Terrain new_chunk = gen_perlin_chunk(world_x, world_z);
                active_chunks.insert({chunk_pos, std::move(new_chunk)});
                Material &mat = new_chunk.get_model().materials[0];
                mat.shader = shader;
                mat.maps[MATERIAL_MAP_OCCLUSION].texture = mraTexture;
                mat.maps[MATERIAL_MAP_ALBEDO].texture = albedoTexture;
                mat.maps[MATERIAL_MAP_NORMAL].texture = normalTexture;
            }
        }
    }
    // De-allocazione chunk oltre RENDER_DISTANCE
    for (auto it = active_chunks.begin(); it != active_chunks.end();) {
        int cx = it->first.first;
        int cz = it->first.second;

        int dist_x = abs(cx - cam_chunk_x);
        int dist_z = abs(cz - cam_chunk_z);

        if (dist_x > RENDER_DISTANCE + 1 || dist_z > RENDER_DISTANCE + 1) {
            it = active_chunks.erase(it);
        } else {
            ++it;
        }
    }
}
