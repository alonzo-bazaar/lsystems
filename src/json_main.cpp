#include <iostream>
#include <cmath>

#include "raylib.h"

#include "lsystem_json.hpp"
#include "turtle.hpp"
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
Model
gen_tree_model(unsigned int r_seed, std::map<char, RewriteTarget> rewrites) {
    srand(r_seed);
    std::vector<instruction> turtle_instructions =
        rewrite_times(7, {{'A',{}}}, rewrites);
    Turtle turtle
        (map_range<float>(0.06f, 0.015f, 7),
         map_range(std::array{0.05f, 0.05f},
             std::array{0.95f, 0.95f},
             7));	
    Model tree_model =
        turtle.follow_instruction_vector(turtle_instructions);
    Image tree_col_im =
        GenImageGradientLinear(10, 10, 0,
                BROWN,
                LIME);
    Texture tree_col_tex = LoadTextureFromImage(tree_col_im);
    UnloadImage(tree_col_im);
    tree_model.materials[0].maps[MATERIAL_MAP_ALBEDO].texture =
        tree_col_tex;
    GenTextureMipmaps(&tree_model
            .materials[0]
            .maps[MATERIAL_MAP_ALBEDO]
            .texture);
    SetTextureFilter(tree_model.materials[0]
            .maps[MATERIAL_MAP_ALBEDO]
            .texture,
            TEXTURE_FILTER_TRILINEAR);

    return tree_model;
}

int main() {
    constexpr float stride = 0.3f;
    constexpr float angle = deg_to_rad(22.5);

    const std::map<char, RewriteTarget> tree_rewrite_rules = {
        RWP('A', [stride, angle](const std::vector<float>& ignored) {
                (void)ignored;
                return std::vector<instruction>
                {{'[',{}},
                {'&',{angle}},
                {'F',{stride}},
                {'L',{}},
                {'!',{}},
                {'A',{}},
                {']',{}},

                {'/',{angle*5}},
                {'\'',{}},

                {'[',{}},
                {'&',{angle}},
                {'F',{stride}},
                {'L',{}},
                {'!',{}},
                {'A',{}},
                {']',{}},

                {'/',{7*angle}},
                {'\'',{}},

                {'[',{}},
                {'&',{angle}},
                {'F',{stride}},
                {'L',{}},
                {'!',{}},
                {'A',{}},
                {']',{}}};
        }),
        RWP('F',
                {{0.9,
                [stride, angle](const std::vector<float>&ignored) {
                (void)ignored;
                return std::vector<instruction>
                {{'S', {}},
                {'/', {4*angle}},
                {'F', {2*stride}}};
                }},
                {0.1,
                [stride, angle](const std::vector<float>&ignored) {
                (void)ignored;
                return std::vector<instruction>
                {{'S', {}},
                {'/', {5*angle}},
                {'F', {stride}}};
                }}}),
        RWP('S',
                [stride, angle](const std::vector<float>&ignored) {
                (void)ignored;
                return std::vector<instruction>
                {{'F', {stride}},
                {'L', {}}};
                }),
        RWP('L',
                [stride, angle](const std::vector<float>&ignored) {
                (void)ignored;
                return std::vector<instruction>
                {{'[',{}},
                {'\'',{}},
                {'\'',{}},
                {'\'',{}},
                {'^', {2*angle}},
                {'{',{}},
                {'-',{angle}},
                {'f',{stride}},
                {'+',{angle}},
                {'f',{stride}},
                {'+',{angle}},
                {'f',{stride}},
                {'-',{angle}},
                {'|',{}},
                {'-',{angle}},
                {'f',{stride}},
                {'+',{angle}},
                {'f',{stride}},
                {'+',{angle}},
                {'f',{stride}},
                {'}',{}},
                {']',{}}};
                })};

    SetConfigFlags(FLAG_MSAA_4X_HINT);
    InitWindow(1280, 960, "L Systems");

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

    auto l = from_json("resources/systems/tree.json");

    Model floor_model = gen_floor_model();
    Model tree_model = gen_tree_model(time(0), tree_rewrite_rules);

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
