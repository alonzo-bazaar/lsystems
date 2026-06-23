//
// Created by Notebook on 22/06/2026.
//
#include <terrain.hpp>

Terrain Terrain::loadFromFile(const char* filename) {
    int bytesLetti = 0;
    unsigned char* fileData = LoadFileData(filename, &bytesLetti);

    int totaleFloat = bytesLetti / sizeof(float);
    int mapSize = static_cast<int>(std::sqrt(totaleFloat));

    mapSize -= 1;

    float* arrayFloatGrezzo = (float*)fileData;
    std::vector<Vector3> verticiTemporanei;
    verticiTemporanei.reserve(totaleFloat);

    for (int r = 0; r < mapSize; r++) {
        for (int c = 0; c < mapSize; c++) {
            Vector3 p;
            p.x = static_cast<float>(c);
            p.y = arrayFloatGrezzo[r * (mapSize+1) + c];
            p.z = static_cast<float>(r);
            verticiTemporanei.push_back(p);
        }
    }

    UnloadFileData(fileData);

    // Restituisce il nuovo oggetto allocato
    return Terrain(std::move(verticiTemporanei), mapSize, mapSize);
}

void Terrain::genTerrainMesh() {
    Mesh mesh = { 0 };
    mesh.vertexCount = terrainVertex.size();
    int numQuadrati = (width - 1) * (width - 1);
    mesh.triangleCount = numQuadrati * 2;

    // 1. ALLOCAZIONE BUFFER (Allochiamo manualmente anche le normali)
    mesh.vertices = (float*)MemAlloc(mesh.vertexCount * 3 * sizeof(float));
    mesh.texcoords = (float*)MemAlloc(mesh.vertexCount * 2 * sizeof(float));
    mesh.normals = (float*)MemAlloc(mesh.vertexCount * 3 * sizeof(float));

    float invWidthMinusOne = 1.0f / (float)(width - 1);

    // 2. RIEMPIMENTO VERTICI E UV
    for (int i = 0; i < mesh.vertexCount; i++) {
        mesh.vertices[i * 3 + 0] = terrainVertex[i].x;
        mesh.vertices[i * 3 + 1] = terrainVertex[i].y;
        mesh.vertices[i * 3 + 2] = terrainVertex[i].z;

        int r = i / width;
        int c = i % width;
        mesh.texcoords[i * 2 + 0] = (float)c * invWidthMinusOne;
        mesh.texcoords[i * 2 + 1] = (float)r * invWidthMinusOne;
    }

    // 3. CALCOLO MANUALE DELLE NORMALI (Genera le sfumature sulle colline)
    for (int r = 0; r < width; r++) {
        for (int c = 0; c < width; c++) {
            int i = r * width + c;

            // Troviamo i vertici vicini (destra, sinistra, sopra, sotto) con controllo dei bordi
            Vector3 attuale = terrainVertex[i];
            Vector3 sinistra = (c > 0) ? terrainVertex[i - 1] : attuale;
            Vector3 destra   = (c < width - 1) ? terrainVertex[i + 1] : attuale;
            Vector3 sopra    = (r > 0) ? terrainVertex[i - width] : attuale;
            Vector3 sotto    = (r < width - 1) ? terrainVertex[i + width] : attuale;

            // Calcoliamo i vettori di pendenza (Tangente e Bitangente)
            Vector3 tangente = { destra.x - sinistra.x, destra.y - sinistra.y, destra.z - sinistra.z };
            Vector3 bitangente = { sotto.x - sopra.x, sotto.y - sopra.y, sotto.z - sopra.z };

            // Il prodotto vettoriale (Cross Product) ci dà la normale ortogonale alla superficie
            Vector3 normale = Vector3CrossProduct(bitangente, tangente);
            normale = Vector3Normalize(normale);

            // Salviamo la normale nel buffer della mesh
            mesh.normals[i * 3 + 0] = normale.x;
            mesh.normals[i * 3 + 1] = normale.y;
            mesh.normals[i * 3 + 2] = normale.z;
        }
    }

    // 4. RIEMPIMENTO INDICI DEI TRIANGOLI
    mesh.indices = (unsigned short*)MemAlloc(mesh.triangleCount * 3 * sizeof(unsigned short));
    int counterIndici = 0;
    for (int r = 0; r < width - 1; r++) {
        for (int c = 0; c < width - 1; c++) {
            int inAltoASinistra = r * width + c;
            int inAltoADestra    = inAltoASinistra + 1;
            int inBassoASinistra = (r + 1) * width + c;
            int inBassoADestra   = inBassoASinistra + 1;

            mesh.indices[counterIndici++] = inAltoASinistra;
            mesh.indices[counterIndici++] = inBassoASinistra;
            mesh.indices[counterIndici++] = inAltoADestra;

            mesh.indices[counterIndici++] = inAltoADestra;
            mesh.indices[counterIndici++] = inBassoASinistra;
            mesh.indices[counterIndici++] = inBassoADestra;
        }
    }

    // Ora che le normali sono calcolate alla perfezione, possiamo generare anche le tangenti se serve!
    GenMeshTangents(&mesh);

    UploadMesh(&mesh, false);
    terrainModel = LoadModelFromMesh(mesh);
    }