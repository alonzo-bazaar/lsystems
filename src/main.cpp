#include<iostream>
#include<vector>
#include<string>
#include<stack>
#include<cmath>
#include<cassert>
#include<initializer_list>

#include "raylib.h"

#include "rewrite.hpp"
#include "turtle.hpp"
#include "light.hpp"

float deg_to_rad(const float deg) {
    return (deg * PI) / 180.0f;
}

template<typename T>
std::vector<T> map_range(T start, T end, size_t n) {
    std::vector<T> res(n);
    double start_weight = 1.0;
    double end_weight = 0.0;
    double step = 1.0/(n-1);

    for(size_t i = 0; i<n; ++i) {
		res[i] = static_cast<T>((start * start_weight) + (end * end_weight));
		start_weight -= step;
		end_weight += step;
    }
    return res;
}

template<>
std::vector<Color> map_range<Color>(Color start, Color end, size_t n) {
    std::vector<int> r = map_range<int>(start.r, end.r, n);
    std::vector<int> g = map_range<int>(start.g, end.g, n);
    std::vector<int> b = map_range<int>(start.b, end.b, n);
    std::vector<int> a = map_range<int>(start.a, end.a, n);
    std::vector<Color> res(n);

    for(size_t i = 0; i<n; ++i) {
		res[i].r = r[i];
		res[i].g = g[i];
		res[i].b = b[i];
		res[i].a = a[i];
    }
    return res;
}

/*
int main() {
    InitWindow(100, 100, "Hello World");
    std::string hilbert_axiom = "A";
    std::map<char, RewriteTarget> hilbert_trans {
		RWP('A' ,"B-F+CFC+F-D&F^D-F+&&CFC+F+B//"),
		RWP('B' ,"A&F^CFB^F^D^^-F-D^|F^B|FC^F^A//"),
		RWP('C' ,"|D^|F^B-F+C^F^A&&FA&F^C+F+B^F^D//"),
		RWP('D' ,"|CFB-F+B|FA&F^A&&FB-F+B|FC//"),
    };
    std::string hilbert = rewrite_times(2, hilbert_axiom, hilbert_trans);
    Turtle t(PI/2, 1.0f, {0.1f}, {LIGHTGRAY});
    std::string& target = hilbert;

	Model tree_model = t.follow_string(target);
	return 0;
}
*/

int main() {
    // questa prima versione è un mix dell'esempio che ti ho mandato su
    // discord e di questo esempio di raylib
    // https://github.com/raysan5/raylib/blob/master/examples/core/core_3d_camera_free.c
    // e sotto ti ho lasciato il link dove trovi la versione eseguibile da web
    // dell'esempio
    // (è la pagina esempi che ho fatto vedere al berretti, se vai un po' sotto
    //  trovi gli esempi 3d)
    // https://www.raylib.com/examples.html
    SetConfigFlags(FLAG_MSAA_4X_HINT);
    InitWindow(1280, 960, "Hello World");

    int target_fps = 60;
    SetTargetFPS(target_fps);

    Camera camera;
    camera.position = (Vector3){ 10.0f, 2.0f, 10.0f };
    camera.target = (Vector3){ 0.0f, 0.0f, 0.0f };
    camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };
    camera.fovy = 45.0f;
    camera.projection = CAMERA_PERSPECTIVE;

    // Load depth shader and get depth texture shader location
    Shader shader = LoadShader(TextFormat("resources/shaders/pbr.vs"), TextFormat("resources/shaders/pbr.fs"));
    shader.locs[SHADER_LOC_MAP_NORMAL] = GetShaderLocation(shader, "normalMap");
    shader.locs[SHADER_LOC_COLOR_DIFFUSE] = GetShaderLocation(shader, "albedoColor");
    shader.locs[SHADER_LOC_VECTOR_VIEW] = GetShaderLocation(shader, "viewPos");
    shader.locs[SHADER_LOC_MAP_OCCLUSION] = GetShaderLocation(shader, "mraMap");

    Image imgRoughness = LoadImage("resources/textures/Grass001_2K-PNG_Roughness.png");
    Image imgAO = LoadImage("resources/textures/Grass001_2K-PNG_AmbientOcclusion.png");

    ImageFormat(&imgRoughness, PIXELFORMAT_UNCOMPRESSED_R8G8B8A8);
    ImageFormat(&imgAO, PIXELFORMAT_UNCOMPRESSED_R8G8B8A8);

    Image imgMRA = GenImageColor(imgRoughness.width, imgRoughness.height, BLACK);
    ImageFormat(&imgMRA, PIXELFORMAT_UNCOMPRESSED_R8G8B8A8);

    Color *pixelsMRA   = (Color *)imgMRA.data;
    Color *pixelsRough = (Color *)imgRoughness.data;
    Color *pixelsAO    = (Color *)imgAO.data;

    int totalPixels = imgMRA.width * imgMRA.height;
    for (int i = 0; i < totalPixels; i++)
		{
			pixelsMRA[i].r = 0;               // R (Metalness)
			pixelsMRA[i].g = pixelsRough[i].r; // G (Roughness)
			pixelsMRA[i].b = pixelsAO[i].r;    // B (Ambient Occlusion)
			pixelsMRA[i].a = 255;             // A (Alpha)
		}

    Texture2D mraTexture = LoadTextureFromImage(imgMRA);
    GenTextureMipmaps(&mraTexture);
    SetTextureFilter(mraTexture, TEXTURE_FILTER_ANISOTROPIC_4X);

    UnloadImage(imgMRA);
    UnloadImage(imgRoughness);
    UnloadImage(imgAO);

    //Image displacementImg = LoadImage("resources/textures/Grass001_2K-PNG_Displacement.png");
    //shader.locs[SHADER_LOC_MAP_HEIGHT] = GetShaderLocation(shader, "resources/textures/Grass001_2K-PNG_Displacement.png");
    //Mesh floorMesh = GenMeshHeightmap(displacementImg, (Vector3){ 100.0f, 0.002f, 100.0f });
    //UnloadImage(displacementImg);

    Mesh floorMesh = GenMeshPlane(100.0f, 100.0f, 100, 100);
    GenMeshTangents(&floorMesh);
    Model floor = LoadModelFromMesh(floorMesh);

    floor.materials[0].shader = shader;

    floor.materials[0].maps[MATERIAL_MAP_OCCLUSION].texture = mraTexture;
    GenTextureMipmaps(&floor.materials[0].maps[MATERIAL_MAP_OCCLUSION].texture);
    SetTextureFilter(floor.materials[0].maps[MATERIAL_MAP_OCCLUSION].texture, TEXTURE_FILTER_TRILINEAR);

    Texture2D albedoTexture = LoadTexture("resources/textures/Grass001_2K-PNG_Color.png");
    floor.materials[0].maps[MATERIAL_MAP_ALBEDO].texture = albedoTexture;
    GenTextureMipmaps(&floor.materials[0].maps[MATERIAL_MAP_ALBEDO].texture);
    SetTextureFilter(floor.materials[0].maps[MATERIAL_MAP_ALBEDO].texture, TEXTURE_FILTER_TRILINEAR);

    Texture2D normalTexture = LoadTexture("resources/textures/Grass001_2K-PNG_NormalGL.png");
    floor.materials[0].maps[MATERIAL_MAP_NORMAL].texture = normalTexture;
    GenTextureMipmaps(&floor.materials[0].maps[MATERIAL_MAP_NORMAL].texture);
    SetTextureFilter(floor.materials[0].maps[MATERIAL_MAP_NORMAL].texture, TEXTURE_FILTER_TRILINEAR);

    int maxLightCount = 1;
    int useTexAlbedo = 1;
    int useTexNormal = 0;
    int useTexMRA    = 1;
    float floorMetallic = 0.0f;
    float floorRoughness = 0.0f;
    float floorAo = 0.0f;
    float floorEmissivePower = 0.0f;
    float ambientIntensity = 0.1f;
    Color ambientColor = { 26, 32, 135, 255 };
    Vector3 ambientColorNormalized = { ambientColor.r/255.0f, ambientColor.g/255.0f, ambientColor.b/255.0f };
    Vector4 floorEmissiveColor = ColorNormalize(floor.materials[0].maps[MATERIAL_MAP_EMISSION].color);
    Vector2 floorTextureTiling = { 20.0f, 20.0f };


    SetShaderValue(shader, GetShaderLocation(shader, "numOfLights"), &maxLightCount, SHADER_UNIFORM_INT);
    SetShaderValue(shader, GetShaderLocation(shader, "useTexAlbedo"), &useTexAlbedo, SHADER_UNIFORM_INT);
    SetShaderValue(shader, GetShaderLocation(shader, "useTexNormal"), &useTexNormal, SHADER_UNIFORM_INT);
    SetShaderValue(shader, GetShaderLocation(shader, "useTexMRA"),    &useTexMRA,    SHADER_UNIFORM_INT);
    SetShaderValue(shader, GetShaderLocation(shader, "metallicValue"), &floorMetallic, SHADER_UNIFORM_FLOAT);
    SetShaderValue(shader, GetShaderLocation(shader, "roughnessValue"), &floorRoughness, SHADER_UNIFORM_FLOAT);
    SetShaderValue(shader, GetShaderLocation(shader, "aoValue"), &floorAo, SHADER_UNIFORM_FLOAT);
    SetShaderValue(shader, GetShaderLocation(shader, "emissivePower"), &floorEmissivePower, SHADER_UNIFORM_FLOAT);
    SetShaderValue(shader, GetShaderLocation(shader, "ambient"), &ambientIntensity, SHADER_UNIFORM_FLOAT);
    SetShaderValue(shader, GetShaderLocation(shader, "ambientColor"), &ambientColorNormalized, SHADER_UNIFORM_VEC3);
    SetShaderValue(shader, GetShaderLocation(shader, "emissiveColor"), &floorEmissiveColor, SHADER_UNIFORM_VEC4);
    SetShaderValue(shader, GetShaderLocation(shader, "tiling"), &floorTextureTiling, SHADER_UNIFORM_VEC2);

    // Create light
    Light sunLight;
    Color sunColor = { 255, 244, 214, 255 };
    sunLight = CreateLight(LIGHT_DIRECTIONAL, { 0.0f, 200.0f, 0.0f }, { 0.0f, 0.0f, 0.0f }, sunColor, 5.0f, shader);
    UpdateLight(shader, sunLight);

	/*
    std::string hilbert_axiom = "A";
    std::map<char, RewriteTarget> hilbert_trans {
		RWP('A' ,"B-F+CFC+F-D&F^D-F+&&CFC+F+B//"),
		RWP('B' ,"A&F^CFB^F^D^^-F-D^|F^B|FC^F^A//"),
		RWP('C' ,"|D^|F^B-F+C^F^A&&FA&F^C+F+B^F^D//"),
		RWP('D' ,"|CFB-F+B|FA&F^A&&FB-F+B|FC//"),
    };
    std::string hilbert = rewrite_times(1, hilbert_axiom, hilbert_trans);
    Turtle t(PI/2, 1.0f, {0.2f}, {LIGHTGRAY});
    std::string& target = hilbert;
	*/

    std::string tree_axiom = "A";

    std::map<char, RewriteTarget> tree_trans = {
		RWP('A', "[&FL!A]/////'[&FL!A]///////'[&FL!A]"),
		RWP('F', {{0.1, "S ///// FF"},
				  {0.3, "S //// F"},
				  {0.6, "S ///// F"}}),
		RWP('S', "F L"),
		RWP('L', "['''^^{-f+f+f-|-f+f+f}]"),
    };
    srand(time(0));
    std::string tree = rewrite_times(7, tree_axiom, tree_trans);
    Turtle t(deg_to_rad(22.5), 0.30f,
			 map_range<float>(0.06f, 0.015f, 7),
			 map_range<Color>(BROWN, GREEN, 7));
    std::string& target = tree;

	Model tree_model = t.follow_string(target);

    while(!WindowShouldClose()) {
		UpdateCamera(&camera, CAMERA_FIRST_PERSON);

		// Update the shader with the camera view vector
		// (points towards { 0.0f, 0.0f, 0.0f })
		float cameraPos[3] = {camera.position.x,
							  camera.position.y,
							  camera.position.z};
		SetShaderValue(shader,
					   shader.locs[SHADER_LOC_VECTOR_VIEW],
					   cameraPos,
					   SHADER_UNIFORM_VEC3);

		if (IsKeyPressed(KEY_Z))
			camera.target = (Vector3){ 0.0f, 0.0f, 0.0f};

		// Check key input to enable/disable sun
		if (IsKeyPressed(KEY_ONE)) {
			sunLight.enabled = !sunLight.enabled;
			UpdateLight(shader, sunLight);
		}

		BeginDrawing(); {
			ClearBackground(BLACK);

			BeginMode3D(camera); {
				DrawModel(floor, (Vector3){ 0.0f, 0.0f, 0.0f }, 1.0f, WHITE);
				// Draw sphere to show the sun position
				if (sunLight.enabled)
					DrawSphereEx(sunLight.position, 0.2f, 8, 8, sunColor);
				DrawModel(tree_model, {0.0f, 0.0f, 0.0f}, 1.0f, WHITE);
			}
			EndMode3D();
			DrawText(TextFormat("FPS: %i (target %i)", GetFPS(), target_fps),
					 10, 10, 20, DARKGRAY);
		} EndDrawing();
    }
    UnloadTexture(mraTexture);
    UnloadTexture(albedoTexture);
    UnloadTexture(normalTexture);
    UnloadShader(shader);      // Unload shader
    UnloadModel(floor);
    CloseWindow();
    return 0;
}
