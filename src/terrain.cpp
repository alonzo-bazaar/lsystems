//
// Created by Notebook on 22/06/2026.
//
#include <terrain.hpp>

// Genera un chunk con coordinate che partono da offset con grandezza size usando perlin
Terrain Terrain::gen_perlin_chunk(const int offset_x, const int offset_z, const int size) {
    std::vector<Vector3> vertices;
    vertices.resize(size * size);

    int x = offset_x;
    for (int r = 0; r < size; r++, x++) {
        int z = offset_z;
        for (int c = 0; c < size; c++, z++) {

            float amplitude = 100.0f;
            float scale = 0.006f;
            int index = r * size + c;

            vertices[index].x = static_cast<float>(z);
            vertices[index].z = static_cast<float>(x);

            float val = perlin_fractal(static_cast<float>(z) * scale, static_cast<float>(x) * scale, 8, 0.5f, 2.0f);
            vertices[index].y = val * amplitude;
        }
    }
    Terrain chunk(std::move(vertices), size, size);
    chunk.gen_chunk_mesh();
    return chunk;
}


void Terrain::gen_chunk_mesh() {
    Mesh mesh = {};
    mesh.vertexCount = width * width;
    int num_squares = (width - 1) * (width - 1);
    mesh.triangleCount = num_squares * 2;

    // Allocazione buffer
    mesh.vertices = static_cast<float *>(MemAlloc(mesh.vertexCount * 3 * sizeof(float)));
    mesh.texcoords = static_cast<float *>(MemAlloc(mesh.vertexCount * 2 * sizeof(float)));
    mesh.normals = static_cast<float *>(MemAlloc(mesh.vertexCount * 3 * sizeof(float)));
    mesh.indices = static_cast<unsigned short *>(MemAlloc(mesh.triangleCount * 3 * sizeof(unsigned short)));

    float inv_width_minus_one = 1.0f / static_cast<float>(width - 1);

    // Assegnazione valori a mesh
    for (int r = 0; r < width; r++) {
        for (int c = 0; c < width; c++) {
            int i = r * width + c;

            // Vertici
            mesh.vertices[i * 3 + 0] = terrain_vertex[i].x;
            mesh.vertices[i * 3 + 1] = terrain_vertex[i].y;
            mesh.vertices[i * 3 + 2] = terrain_vertex[i].z;

            // UV
            mesh.texcoords[i * 2 + 0] = static_cast<float>(c) * inv_width_minus_one;
            mesh.texcoords[i * 2 + 1] = static_cast<float>(r) * inv_width_minus_one;

            // Calcolo normali
            Vector3 current = terrain_vertex[i];
            Vector3 left    = (c > 0) ? terrain_vertex[i - 1] : current;
            Vector3 right   = (c < width - 1) ? terrain_vertex[i + 1] : current;
            Vector3 top     = (r > 0) ? terrain_vertex[i - width] : current;
            Vector3 bottom  = (r < width - 1) ? terrain_vertex[i + width] : current;

            Vector3 tangent = { right.x - left.x, right.y - left.y, right.z - left.z };
            Vector3 bitangent = { bottom.x - top.x, bottom.y - top.y, bottom.z - top.z };

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
    for (int r = 0; r < width - 1; r++) {
        for (int c = 0; c < width - 1; c++) {
            int top_left = r * width + c;
            int top_right    = top_left + 1;
            int bottom_left = (r + 1) * width + c;
            int bottom_right   = bottom_left + 1;

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

float interpolate(float a0, float a1, float w)
{
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
float Terrain::perlin_fractal(float x, float y, int octaves, float persistence, float lacunarity) {
    float total = 0.0f;
    float frequency = 1.0f;
    float amplitude = 1.0f;
    float max_value = 0.0f; // Usato per normalizzare il risultato finale

    for (int i = 0; i < octaves; i++) {
        total += perlin(x * frequency, y * frequency) * amplitude;

        max_value += amplitude;

        amplitude *= persistence;    // Diminuisce l'impatto dei dettagli piccoli
        frequency *= lacunarity;     // Aumenta la frequenza (il dettaglio)
    }

    return total / max_value; // Restituisce un valore tra -1 e 1
}