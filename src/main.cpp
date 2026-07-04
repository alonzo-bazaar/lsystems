#include<iostream>
#include<vector>
#include<string>
#include<stack>
#include<cmath>
#include<cassert>
#include<initializer_list>

#include "raylib.h"

#include "raymath.h"
#include "rewrite.hpp"
#include "turtle.hpp"
#include "light.hpp"
#include "player.hpp"
#include "terrain.hpp"
#include "utils.hpp"
#include "lsystem.hpp"

int main() {
    SetConfigFlags(FLAG_MSAA_4X_HINT);
    InitWindow(1280, 960, "L Systems");
    DisableCursor();
    int target_fps = 60;
    //SetTargetFPS(target_fps);

    // get trees from json

    // Setup camera
    Player player({0.0f, (0.5f + 1.0f), 0.0f});

    // Setup shader
    Shader shader = LoadShader(TextFormat("resources/shaders/pbr.vs"), TextFormat("resources/shaders/pbr.fs"));
    shader.locs[SHADER_LOC_MAP_NORMAL] = GetShaderLocation(shader, "normalMap");
    shader.locs[SHADER_LOC_COLOR_DIFFUSE] = GetShaderLocation(shader, "albedoColor");
    shader.locs[SHADER_LOC_VECTOR_VIEW] = GetShaderLocation(shader, "viewPos");
    shader.locs[SHADER_LOC_MAP_OCCLUSION] = GetShaderLocation(shader, "mraMap");

    // Setup MRA per shader mettendo insieme roughness e AO
    Image imgRoughness = LoadImage("resources/textures/Grass007_2K-PNG_Roughness.png");
    Image imgAO = LoadImage("resources/textures/Grass007_2K-PNG_AmbientOcclusion.png");
    ImageFormat(&imgRoughness, PIXELFORMAT_UNCOMPRESSED_R8G8B8A8);
    ImageFormat(&imgAO, PIXELFORMAT_UNCOMPRESSED_R8G8B8A8);
    Image imgMRA = GenImageColor(imgRoughness.width, imgRoughness.height, BLACK);
    ImageFormat(&imgMRA, PIXELFORMAT_UNCOMPRESSED_R8G8B8A8);

    Color *pixelsMRA = (Color *) imgMRA.data;
    Color *pixelsRough = (Color *) imgRoughness.data;
    Color *pixelsAO = (Color *) imgAO.data;

    int totalPixels = imgMRA.width * imgMRA.height;
    for (int i = 0; i < totalPixels; i++) {
        pixelsMRA[i].r = 0;                // R (Metalness)
        pixelsMRA[i].g = pixelsRough[i].r; // G (Roughness)
        pixelsMRA[i].b = pixelsAO[i].r;    // B (Ambient Occlusion)
        pixelsMRA[i].a = 255;              // A (Alpha)
    }

    Texture2D mraTexture = LoadTextureFromImage(imgMRA);
    GenTextureMipmaps(&mraTexture);
    SetTextureFilter(mraTexture, TEXTURE_FILTER_ANISOTROPIC_4X);

    UnloadImage(imgMRA);
    UnloadImage(imgRoughness);
    UnloadImage(imgAO);

    // Setup texture terreno
    Texture2D albedoTexture = LoadTexture("resources/textures/Grass007_2K-PNG_Color.png");
    Texture2D normalTexture = LoadTexture("resources/textures/Grass007_2K-PNG_NormalGL.png");

    auto setupTexture = [](Texture2D &tex) {
        GenTextureMipmaps(&tex);
        SetTextureFilter(tex, TEXTURE_FILTER_TRILINEAR);
    };

    setupTexture(albedoTexture);
    setupTexture(normalTexture);
    setupTexture(mraTexture);

    // Setup variabili shader
    int maxLightCount = 1;
    int useTexAlbedo = 1;
    int useTexNormal = 1;
    int useTexMRA = 1;
    float floorMetallic = 0.2f;
    float floorRoughness = 0.8f;
    float floorAo = 0.5f;
    float floorEmissivePower = 0.0f;
    float ambientIntensity = 0.1f;
    Color ambientColor = {26, 32, 135, 255};
    Vector3 ambientColorNormalized = {
        ambientColor.r / 255.0f,
        ambientColor.g / 255.0f,
        ambientColor.b / 255.0f
    };

    SetTextureWrap(albedoTexture, TEXTURE_WRAP_REPEAT);
    SetTextureWrap(normalTexture, TEXTURE_WRAP_REPEAT);
    SetTextureWrap(mraTexture, TEXTURE_WRAP_REPEAT);

    SetShaderValue(shader, GetShaderLocation(shader, "numOfLights"),
                   &maxLightCount, SHADER_UNIFORM_INT);
    SetShaderValue(shader, GetShaderLocation(shader, "useTexAlbedo"),
                   &useTexAlbedo, SHADER_UNIFORM_INT);
    SetShaderValue(shader, GetShaderLocation(shader, "useTexNormal"),
                   &useTexNormal, SHADER_UNIFORM_INT);
    SetShaderValue(shader, GetShaderLocation(shader, "useTexMRA"),
                   &useTexMRA, SHADER_UNIFORM_INT);
    SetShaderValue(shader, GetShaderLocation(shader, "metallicValue"),
                   &floorMetallic, SHADER_UNIFORM_FLOAT);
    SetShaderValue(shader, GetShaderLocation(shader, "roughnessValue"),
                   &floorRoughness, SHADER_UNIFORM_FLOAT);
    SetShaderValue(shader, GetShaderLocation(shader, "aoValue"),
                   &floorAo, SHADER_UNIFORM_FLOAT);
    SetShaderValue(shader, GetShaderLocation(shader, "emissivePower"),
                   &floorEmissivePower, SHADER_UNIFORM_FLOAT);
    SetShaderValue(shader, GetShaderLocation(shader, "ambient"),
                   &ambientIntensity, SHADER_UNIFORM_FLOAT);
    SetShaderValue(shader, GetShaderLocation(shader, "ambientColor"),
                   &ambientColorNormalized, SHADER_UNIFORM_VEC3);

    // Crea sole
    Color sunColor = {255, 244, 214, 255};
    Light sunLight = CreateLight(LIGHT_DIRECTIONAL, {0.0f, 1050.0f, 1050.0f},
                                 {0.0f, 0.0f, 0.0f}, sunColor, 5.0f, shader);

    std::vector<std::pair<Vector3, Model> > tree_positions;

    std::map<std::pair<int, int>, Terrain> active_chunks;

    while (!WindowShouldClose()) {
        int centerX = GetScreenWidth() / 2;
        int centerY = GetScreenHeight() / 2;

        char sideway = IsKeyDown(KEY_D) - IsKeyDown(KEY_A);
        char forward = IsKeyDown(KEY_W) - IsKeyDown(KEY_S);
        bool crouching = IsKeyDown(KEY_LEFT_CONTROL);
        bool jumping = IsKeyPressed(KEY_SPACE);

        player.update_body(sideway, forward, jumping, crouching);

        player.update_camera_first_person();

        // Aggiorna la posizione della camera nello shader
        player.update_shader_position(shader);

        // Generazione terreno
        Terrain::chunk_management(active_chunks, player.get_camera(), shader, mraTexture, albedoTexture, normalTexture);

        // Reset target camera
        if (IsKeyPressed(KEY_Z))
            player.set_position({0.0f, 0.0f, 0.0f});

        // Controlla click mouse sinistro per aggiungere albero
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            Vector2 screenCenter = {static_cast<float>(centerX), static_cast<float>(centerY)};
            Ray crosshairRay = GetMouseRay(screenCenter, player.get_camera());
            // Verifica se raggio colpisce bbox del chunk
            for (auto &chunk: active_chunks) {
                BoundingBox box = GetMeshBoundingBox(chunk.second.get_model().meshes[0]);

                box.min.x += chunk.second.get_world_x();
                box.min.z += chunk.second.get_world_z();
                box.max.x += chunk.second.get_world_x();
                box.max.z += chunk.second.get_world_z();

                if (GetRayCollisionBox(crosshairRay, box).hit) {
                    Matrix chunkTransform = MatrixTranslate(chunk.second.get_world_x(), 0.0f,
                                                            chunk.second.get_world_z());
                    RayCollision col = GetRayCollisionMesh(crosshairRay,
                                                           chunk.second.get_model().meshes[0], chunkTransform);
                    // Se colpisce terreno entro minDistance aggiunge albero
                    if (float minDistance = 15.0f; col.hit && col.distance < minDistance) {
                        tree_positions.emplace_back(col.point, gen_tree_model(time(0), shader));
                        break;
                    }
                }
            }
        }

        BeginDrawing();
        {
            ClearBackground(SKYBLUE);
            BeginMode3D(player.get_camera());
            {
                player.update_frustum();

                // Disegna terreno
                Terrain::draw_visible_chunk(active_chunks, player);

                // Disegna sole
                if (sunLight.enabled)
                    DrawSphere(sunLight.position, 100, sunColor);
                // Disegna alberi
                for (const auto &p: tree_positions) {
                    Vector3 tree_pos = p.first;
                    std::pair tree_chunk = {
                        static_cast<int>(floorf(tree_pos.x / Terrain::CHUNK_SIZE)),
                        static_cast<int>(floorf(tree_pos.z / Terrain::CHUNK_SIZE))
                    };
                    // Se il chunk calcolato è attivo, disegna l'albero
                    if (active_chunks.find(tree_chunk) != active_chunks.end()) {
                        DrawModel(p.second, tree_pos, 1.0f, WHITE);
                    }
                }
            }
            EndMode3D();

            DrawLine(centerX - 20, centerY, centerX + 20, centerY, BLACK);
            DrawLine(centerX, centerY - 20, centerX, centerY + 20, BLACK);
            DrawText(TextFormat("FPS: %i (target %i)", GetFPS(), target_fps),
                     10, 10, 20, DARKGRAY);
        }
        EndDrawing();
    }

    // Unload roba
    UnloadTexture(mraTexture);
    UnloadTexture(albedoTexture);
    UnloadTexture(normalTexture);
    UnloadShader(shader);
    for (auto &terrain: active_chunks)
        UnloadModel(terrain.second.get_model());

    CloseWindow();
    return 0;
}
