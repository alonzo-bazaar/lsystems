//
// Created by Notebook on 22/06/2026.
//
#include <terrain.hpp>

Terrain Terrain::load_from_file(const char* filename) {
    int nread_bytes = 0;
    unsigned char* fileData = LoadFileData(filename, &nread_bytes);

    int nread_floats = nread_bytes / sizeof(float);
    int map_size = static_cast<int>(std::sqrt(nread_floats));

    map_size -= 1;

    float* raw_float_array = (float*)fileData;
    std::vector<Vector3> temp_vertices;
    temp_vertices.reserve(nread_floats);

    for (int r = 0; r < map_size; r++) {
        for (int c = 0; c < map_size; c++) {
            Vector3 p;
            p.x = static_cast<float>(c);
            p.y = raw_float_array[r * (map_size+1) + c];
            p.z = static_cast<float>(r);
            temp_vertices.push_back(p);
        }
    }

    UnloadFileData(fileData);

    // Restituisce il nuovo oggetto allocato
    return Terrain(std::move(temp_vertices), map_size, map_size);
}

void Terrain::gen_terrain_mesh() {
    int n_squares = (width - 1) * (width - 1);

    // crea mesh terreno
    Mesh mesh = {0};
    mesh.vertexCount = terrain_vertices.size();
    mesh.triangleCount = n_squares * 2;
    mesh.vertices = (float*)MemAlloc(mesh.vertexCount * 3 * sizeof(float));
    mesh.texcoords = (float*)MemAlloc(mesh.vertexCount * 2 * sizeof(float));
    mesh.normals = (float*)MemAlloc(mesh.vertexCount * 3 * sizeof(float));

    float inv_w_minus_one = 1.0f / (float)(width - 1);

    // setta texcoord della mesh terreno 
    for (int i = 0; i < mesh.vertexCount; i++) {
        mesh.vertices[i * 3 + 0] = terrain_vertices[i].x;
        mesh.vertices[i * 3 + 1] = terrain_vertices[i].y;
        mesh.vertices[i * 3 + 2] = terrain_vertices[i].z;

        int r = i / width;
        int c = i % width;
        mesh.texcoords[i * 2 + 0] = (float)c * inv_w_minus_one;
        mesh.texcoords[i * 2 + 1] = (float)r * inv_w_minus_one;
    }

    // calcolo normali
    for (int r = 0; r < width; r++) {
        for (int c = 0; c < width; c++) {
            int i = r * width + c;

            // trova tutti i vertici adiacenti, controlla i bordi
            Vector3 curr = terrain_vertices[i];
            Vector3 left = (c > 0) ? terrain_vertices[i - 1] : curr;
            Vector3 right = (c < width - 1) ? terrain_vertices[i + 1] : curr;
            Vector3 up = (r > 0) ? terrain_vertices[i - width] : curr;
            Vector3 down = (r < width - 1) ? terrain_vertices[i + width] : curr;

            // calcola i vettori di pendenza (tangente e bitangente)
            Vector3 tan = {right.x - left.x,
                           right.y - left.y,
                           right.z - left.z
            };
            Vector3 bitan = {down.x - up.x,
                             down.y - up.y,
                             down.z - up.z
            };
            Vector3 norm = Vector3Normalize(Vector3CrossProduct(bitan, tan));

            // Salviamo la norm nel buffer della mesh
            mesh.normals[i * 3 + 0] = norm.x;
            mesh.normals[i * 3 + 1] = norm.y;
            mesh.normals[i * 3 + 2] = norm.z;
        }
    }

    // calcola vertex indices
    mesh.indices = (unsigned short*)calloc(mesh.triangleCount * 3,
                                           sizeof(unsigned short));
    int counter = 0;
    for (int r = 0; r < width - 1; r++) {
        for (int c = 0; c < width - 1; c++) {
            int up_left = r * width + c;
            int up_right    = up_left + 1;
            int down_left = (r + 1) * width + c;
            int down_right   = down_left + 1;

            mesh.indices[counter++] = up_left;
            mesh.indices[counter++] = down_left;
            mesh.indices[counter++] = up_right;

            mesh.indices[counter++] = up_right;
            mesh.indices[counter++] = down_left;
            mesh.indices[counter++] = down_right;
        }
    }

    // calcolate le normali sono state calcolate possiamo generare le tangenti
    GenMeshTangents(&mesh);

    UploadMesh(&mesh, false);
    terrain_model = LoadModelFromMesh(mesh);
}
