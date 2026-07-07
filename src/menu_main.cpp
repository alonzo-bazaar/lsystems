#include "lsystem.hpp"
#include "menu.hpp"

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

std::pair<std::vector<std::string>, std::map<std::string, Lsystem>>
read_the_json(const char* json_filename) {
    // get trees
    auto parsed_r = from_json_file(json_filename);
    if(parsed_r.is_err())
        throw std::runtime_error
            ("error while parsing the json file:\n" + parsed_r.string_trace());

    std::map<std::string, ParsedTree> parsed = parsed_r.get();

    std::vector<std::string> tree_names;
    std::map<std::string, Lsystem> trees;
    for(const auto& [k, v] : parsed) {
        trees.insert({k, Lsystem::from_parsed_tree(v, BROWN, LIME)});
        tree_names.push_back(k);
    }

    if(tree_names.size() == 0)
        throw std::runtime_error
            ("json file contained no trees, no idea what to render now");
    return {tree_names, trees};
}

std::map<std::string, TreeModel>
gen_models_map(const std::map<std::string, Lsystem>& lsystems_map,
               const unsigned int seed) {
    std::map<std::string, TreeModel> models;
    for(const auto& [k, v] : lsystems_map)
        models.insert({k, v.gen_model(seed)});
    return models;
}

int main() {
    // initializing shit
    SetConfigFlags(FLAG_MSAA_4X_HINT);
    InitWindow(1280, 960, "L Systems");
    DisableCursor();
    int target_fps = 60;
    SetTargetFPS(target_fps);

    // get trees and tree models
    auto [tree_names, lsystems_map] = read_the_json("the_json.json");
    std::map<std::string, TreeModel> models_map =
        gen_models_map(lsystems_map, time(0));
    Menu menu = Menu(tree_names);
    // we should also get the floor model
    Model floor_model = gen_floor_model();

    // create camera to initiate horizontally spinning tree
    float camera_radius = 25.0f;
    float camera_height = 30.0f;
    Camera camera;
    camera.position = {camera_radius, camera_height, 0.0f};
    camera.target = {0.0f, 0.0f, 0.0f};
    camera.up = {0.0f, 1.0f, 0.0f};
    camera.fovy = 45.0f;
    camera.projection = CAMERA_PERSPECTIVE;

    while (!WindowShouldClose()) {
        const double t = GetTime();
        camera.position.x = camera_radius * cos(t);
        camera.position.z = camera_radius * sin(t);
        camera.target = (Vector3){ 0.0f, 0.0f, 0.0f};

        BeginDrawing(); {
            ClearBackground(WHITE);

            // r for reload
            if(IsKeyPressed(KEY_R)) {
                try {
                auto p = read_the_json("the_json.json");
                tree_names = p.first; lsystems_map = p.second;

                models_map = gen_models_map(lsystems_map, time(0));
                menu = Menu(tree_names);
                }
                catch(std::exception& e) {
                    std::cerr<< "error occured while reloading:\n"
                             << e.what()
                             << std::endl;
                }
            }

            // menu processes m key to toggle
            // and potentially j, k, and enter keys to navigate
            menu.process_input();

            // draw menu
            menu.draw();

            // spinning tree
            BeginMode3D(camera); {
                models_map.find(menu.current_pick())->second.draw({0.0f, 0.0f, 0.0f});
                DrawModel(floor_model, {0.0f, 0.0f, 0.0f},
                        1.0f, WHITE);
            } EndMode3D();

            DrawText(menu.current_pick().c_str(), 300, 300, 30, BLACK);
        } EndDrawing();
    }
    
    CloseWindow();
    return 0;
}
