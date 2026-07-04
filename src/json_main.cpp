#include <iostream>
#include <cmath>

#include "raylib.h"

//#include "lsystem_json.hpp"
//#include "turtle.hpp"
#include "lsystem.hpp"
#include "utils.hpp"

Model gen_floor_model() {
    // texture ripetuta
    float floor_width = 10.0f;
    float floor_length = 10.0f;
    float floor_vertices[] = {
        +floor_width/2, 0, +floor_length/2,
        +floor_width/2, 0, -floor_length/2,
        -floor_width/2, 0, -floor_length/2,
        -floor_width/2, 0, +floor_length/2,
    };

    float floor_texcoords[] = {
        1.0f, 1.0f,
        1.0f, 0.0f,
        0.0f, 0.0f,
        0.0f, 1.0f,
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

    UploadMesh(&floor_mesh, false);
    Model floor_model = LoadModelFromMesh(floor_mesh);
    floor_model.materials[0].maps[MATERIAL_MAP_ALBEDO].texture =
        LoadTextureFromImage
        (GenImageChecked(10, 10, 1, 1,
                         PURPLE,
                         BLACK));

    return floor_model;
}

// versione ridotta e leggermente aggiornata rispetto a quella
// presente nel main main

Model gen_tree_model(unsigned int r_seed, std::map<char, RewriteTarget> rewrites) {
    Lsystem tree = Lsystem
        (map_range<float>(0.06f, 0.015f, 7),
         map_range<std::array<float, 2>>({0.05f, 0.05f}, {0.95f, 0.95f}, 7),
         vertical_gradient(20, WHITE, BLUE),
         7,
         {{'A', {}}},
         rewrites);

    return tree.gen_model(r_seed);
}

Model gen_tree_model(unsigned int r_seed,
               std::map<char, RewriteTarget> rewrites,
               Shader shader) {
    Model model = gen_tree_model(r_seed, rewrites);
    model.materials[0].shader = shader;
    return model;
}

int main() {
    SetConfigFlags(FLAG_MSAA_4X_HINT);
    InitWindow(1280, 960, "L Systems");

    auto parsed_r = from_json_file("the_json.json");
    if(parsed_r.is_err())
        throw std::runtime_error(parsed_r.string_trace());

    std::map<std::string, ParsedTree> parsed = parsed_r.get();
    std::map<std::string, Lsystem> trees;
    for(const auto& [k, v] : parsed)
        trees.insert({k, Lsystem::from_parsed_tree(v, WHITE, BLUE)});

    auto f = std::ranges::find_if
        (trees,
         [](const std::pair<std::string, Lsystem>& kv) {
             return kv.first != "version";
         });
    
    if(f == trees.end())
        throw std::runtime_error
            ("no trees were provided, the fuck am I supposed to do?");

    auto [name, tree] = *f;

    int target_fps = 60;
    SetTargetFPS(target_fps);

    float camera_radius = 25.0f;
    float camera_height = 30.0f;
    Camera camera;
    camera.position = {camera_radius, camera_height, 0.0f};
    camera.target = {0.0f, 0.0f, 0.0f};
    camera.up = {0.0f, 1.0f, 0.0f};
    camera.fovy = 45.0f;
    camera.projection = CAMERA_PERSPECTIVE;

    Model floor_model = gen_floor_model();
    Model tree_model = tree.gen_model(time(0));

    while(!WindowShouldClose()) {
        // horizontally spinning camera
        double t = GetTime();
        camera.position.x = camera_radius * cos(t);
        camera.position.z = camera_radius * sin(t);
        camera.target = (Vector3){ 0.0f, 0.0f, 0.0f};

        BeginDrawing(); {
            ClearBackground(BLACK);
            BeginMode3D(camera); {
                DrawModel(floor_model, {0.0f, 0.0f, 0.0f},
                        1.0f, WHITE);
                DrawModel(tree_model, {0.0f, 0.0f, 0.0f},
                        1.0f, WHITE);
            } EndMode3D();
        } EndDrawing();
    }
}
