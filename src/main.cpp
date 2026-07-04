#include<iostream>
#include<vector>
#include<string>
#include<stack>
#include<cmath>
#include<cassert>
#include<initializer_list>

#include "raylib.h"

#include "light.hpp"
#include "utils.hpp"
#include "lsystem.hpp"

int main() {
    SetConfigFlags(FLAG_MSAA_4X_HINT);
    InitWindow(1280, 960, "L Systems");

    int target_fps = 60;
    SetTargetFPS(target_fps);

    Camera camera;
    camera.position = (Vector3){ 10.0f, 2.0f, 10.0f };
    camera.target = (Vector3){ 0.0f, 0.0f, 0.0f };
    camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };
    camera.fovy = 45.0f;
    camera.projection = CAMERA_PERSPECTIVE;

    // carica lo shader e setta le posizioni che ci servono all'interno di questo 
    Shader shader = LoadShader("resources/shaders/pbr.vs",
                               "resources/shaders/pbr.fs");
    shader.locs[SHADER_LOC_MAP_NORMAL] = GetShaderLocation(shader, "normalMap");
    shader.locs[SHADER_LOC_COLOR_DIFFUSE] = GetShaderLocation(shader, "albedoColor");
    shader.locs[SHADER_LOC_VECTOR_VIEW] = GetShaderLocation(shader, "viewPos");
    shader.locs[SHADER_LOC_MAP_OCCLUSION] = GetShaderLocation(shader, "mraMap");

    // texture utilizzate per roughness e normal del "pavimento"
    Image imgRoughness = LoadImage("resources/textures/Grass007_2K-PNG_Roughness.png");
    Image imgAO = LoadImage("resources/textures/Grass007_2K-PNG_AmbientOcclusion.png");

    ImageFormat(&imgRoughness, PIXELFORMAT_UNCOMPRESSED_R8G8B8A8);
    ImageFormat(&imgAO, PIXELFORMAT_UNCOMPRESSED_R8G8B8A8);

    Image imgMRA = GenImageColor(imgRoughness.width, imgRoughness.height, BLACK);
    ImageFormat(&imgMRA, PIXELFORMAT_UNCOMPRESSED_R8G8B8A8);

    Color *pixelsMRA   = (Color *)imgMRA.data;
    Color *pixelsRough = (Color *)imgRoughness.data;
    Color *pixelsAO    = (Color *)imgAO.data;

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

    // la mesh del pavimento viene creata manualmente
    // al momento invece di avere un pavimento diviso in più sottomesh
    // impostiamo la texture del pavimento per essere ripetuta a coordinate
    // uv oltre il range (0, 1) - (0, 1), e mettiamo le texture cordinate del
    // pavimento a -10 - 10
    // questo ci da un modo un po' più semplice per avere un pavimento con
    // texture ripetuta
    float floor_width = 100.0f;
    float floor_length = 100.0f;
    float floor_vertices[] = {
        +floor_width/2, 0, +floor_length/2,
        +floor_width/2, 0, -floor_length/2,
        -floor_width/2, 0, -floor_length/2,
        -floor_width/2, 0, +floor_length/2,
    };

    float floor_texcoords[] = {
        +10.0f, +10.0f,
        +10.0f, -10.0f,
        -10.0f, -10.0f,
        -10.0f, +10.0f,
    };

    float floor_normals[] = {
        0.0f, 1.0f, 0.0f,
        0.0f, 1.0f, 0.0f,
        0.0f, 1.0f, 0.0f,
        0.0f, 1.0f, 0.0f,
    };

    unsigned short floor_indices[] = {
        0, 1, 2,
        2, 3, 0,
    };

    Mesh floor_mesh = {0};
    floor_mesh.vertices = floor_vertices;
    floor_mesh.texcoords = floor_texcoords;
    floor_mesh.normals = floor_normals;
    floor_mesh.indices = floor_indices;

    floor_mesh.vertexCount = 4;
    floor_mesh.triangleCount = 2;

    // false è per dire che è una static mesh
    // dynamic=false
    UploadMesh(&floor_mesh, false);

    Model floor = LoadModelFromMesh(floor_mesh);

    floor.materials[0].shader = shader;

    floor.materials[0].maps[MATERIAL_MAP_OCCLUSION].texture = mraTexture;
    GenTextureMipmaps(&floor.materials[0].maps[MATERIAL_MAP_OCCLUSION].texture);
    SetTextureFilter(floor.materials[0].maps[MATERIAL_MAP_OCCLUSION].texture, TEXTURE_FILTER_TRILINEAR);

    Texture2D albedoTexture = LoadTexture("resources/textures/Grass007_2K-PNG_Color.png");
    floor.materials[0].maps[MATERIAL_MAP_ALBEDO].texture = albedoTexture;
    GenTextureMipmaps(&floor.materials[0].maps[MATERIAL_MAP_ALBEDO].texture);
    SetTextureFilter(floor.materials[0].maps[MATERIAL_MAP_ALBEDO].texture, TEXTURE_FILTER_TRILINEAR);

    Texture2D normalTexture = LoadTexture("resources/textures/Grass007_2K-PNG_NormalGL.png");
    floor.materials[0].maps[MATERIAL_MAP_NORMAL].texture = normalTexture;
    GenTextureMipmaps(&floor.materials[0].maps[MATERIAL_MAP_NORMAL].texture);
    SetTextureFilter(floor.materials[0].maps[MATERIAL_MAP_NORMAL].texture, TEXTURE_FILTER_TRILINEAR);

    int maxLightCount = 1;
    int useTexAlbedo = 1;
    int useTexNormal = 0;
    int useTexMRA    = 1;

    float floorMetallic = 0.2f;
    float floorRoughness = 0.8f;
    float floorAo = 0.5f;
    float floorEmissivePower = 0.0f;

    float ambientIntensity = 0.1f;
    Color ambientColor = { 26, 32, 135, 255 };
    Vector3 ambientColorNormalized = { ambientColor.r/255.0f,
                                       ambientColor.g/255.0f,
                                       ambientColor.b/255.0f };

    Vector4 floorEmissiveColor =
        ColorNormalize(floor.materials[0].maps[MATERIAL_MAP_EMISSION].color);

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
    SetShaderValue(shader, GetShaderLocation(shader, "emissiveColor"),
                   &floorEmissiveColor, SHADER_UNIFORM_VEC4);

    // questo valore non è utilizzato, ma l'uniforme `tiling` viene utilizzata
    // dallo shader pbr che abbiamo preso e modificato per questo progetto
    // per adesso la settiamo solo a un valore di "come se non ci fosse"
    // prima di passarla allo shader
    Vector2 floorTextureTiling = {0.5, 0.5f};

    SetShaderValue(shader, GetShaderLocation(shader, "tiling"),
                   &floorTextureTiling, SHADER_UNIFORM_VEC2);

    // e luce fu
    Light sunLight;
    Color sunColor = { 255, 244, 214, 255 };
    sunLight = CreateLight(LIGHT_DIRECTIONAL,
                           { 0.0f, 20.0f, 50.0f }, { 0.0f, 0.0f, 0.0f },
                           sunColor, 5.0f, shader);
    UpdateLight(shader, sunLight);

    // default_cube, lasciato per essere, in caso, scommentato a fini di testing

    // Model default_cube = LoadModelFromMesh(GenMeshCube(1.0f, 2.0f, 1.0f));
    // default_cube.materials[0].maps[MATERIAL_MAP_ALBEDO].texture = 
    // 	LoadTextureFromImage(GenImageChecked(2, 2, 1, 1, BLACK, PURPLE));
    // default_cube.materials[0].maps[MATERIAL_MAP_OCCLUSION].texture = 
    // 	LoadTextureFromImage(GenImageColor(10, 10, {0, 255, 0, 255}));
    // default_cube.materials[0].shader = shader;

    Lsystem basic_tree = basic_tree_lsystem();

    std::vector<std::pair<Vector3, Model>> tree_positions{
        // {{0, 0, 0}, default_cube}
    };
    while(!WindowShouldClose()) {
        int centerX = GetScreenWidth()/2;
        int centerY = GetScreenHeight()/2;

        UpdateCamera(&camera, CAMERA_FIRST_PERSON);

        float cameraPos[3] = {camera.position.x,
                              camera.position.y,
                              camera.position.z};
        SetShaderValue(shader,
                       shader.locs[SHADER_LOC_VECTOR_VIEW],
                       cameraPos,
                       SHADER_UNIFORM_VEC3);

        // premere z resetta la telecamera e la fa puntare verso l'origine
        if (IsKeyPressed(KEY_Z))
            camera.target = (Vector3){ 0.0f, 0.0f, 0.0f};

        // premere 1 accende o spegne il sole
        if (IsKeyPressed(KEY_ONE)) {
            sunLight.enabled = !sunLight.enabled;
            UpdateLight(shader, sunLight);
        }

        // tasto destro per piantare un albero nel punto dove
        // il crosshair al centro dello schermo interseca il pavimento 
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            Vector2 screenCenter = { (float)centerX, (float)centerY };
            Ray crosshairRay = GetMouseRay(screenCenter, camera);
            RayCollision collision = GetRayCollisionMesh(crosshairRay,
                                                         floor_mesh,
                                                         floor.transform);
            if (collision.hit) {
                tree_positions.push_back
                    ({collision.point, basic_tree.gen_model(time(0))});
            }
        }

        BeginDrawing(); {
            ClearBackground(SKYBLUE);

            BeginMode3D(camera); {
                DrawModel(floor, {0.0f, 0.0f, 0.0f}, 1.0f, WHITE);
                // disegna una sfera dove abbiamo messo la light
                // source del sole per indicare la posizione del suddetto
                if (sunLight.enabled)
                    DrawSphereEx(sunLight.position, 0.2f, 8, 8, sunColor);

                for(const auto& p:tree_positions)
                    DrawModel(p.second, p.first, 1.0f, WHITE);
            }
            EndMode3D();

            DrawLine(centerX - 20, centerY, centerX + 20, centerY, BLACK);
            DrawLine(centerX, centerY - 20, centerX, centerY + 20, BLACK);
            DrawText(TextFormat("FPS: %i (target %i)", GetFPS(), target_fps),
                     10, 10, 20, DARKGRAY);

        } EndDrawing();
    }

    UnloadTexture(mraTexture);
    UnloadTexture(albedoTexture);
    UnloadTexture(normalTexture);
    UnloadShader(shader);
    UnloadModel(floor);
    CloseWindow();
    return 0;
}
