#include<iostream>
#include<vector>
#include<string>
#include<stack>
#include<cmath>
#include<initializer_list>

#include "raylib.h"

#include "rewrite.hpp"
#include "geom.hpp"


int main() {
	// questa prima versione è un mix dell'esempio che ti ho mandato su
	// discord e di questo esempio di raylib
	// https://github.com/raysan5/raylib/blob/master/examples/core/core_3d_camera_free.c
	// e sotto ti ho lasciato il link dove trovi la versione eseguibile da web
	// dell'esempio
	// (è la pagina esempi che ho fatto vedere al berretti, se vai un po' sotto
	//  trovi gli esempi 3d)
	// https://www.raylib.com/examples.html

	InitWindow(640, 480, "Hello World");
	Camera camera = { 0 };
    camera.position = (Vector3){ 10.0f, 10.0f, 10.0f };
    camera.target = (Vector3){ 0.0f, 0.0f, 0.0f };
    camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };
    camera.fovy = 45.0f;
    camera.projection = CAMERA_PERSPECTIVE;

    // Vector3 cubePosition = { 0.0f, 0.0f, 0.0f };

	// Model cube = LoadModelFromMesh(GenMeshCube(2.0f, 2.0f, 2.0f));

	Turtle t(PI/2, 0.1f, 0.1f);

	// n=2, δ=90◦
	std::string axiom = "A";
	std::map<char, std::string> transformations {
		{'A' ,"B-F+CFC+F-D&F∧D-F+&&CFC+F+B//"},
		{'B' ,"A&F∧CFB∧F∧D∧∧-F-D∧|F∧B|FC∧F∧A//"},
		{'C' ,"|D∧|F∧B-F+C∧F∧A&&FA&F∧C+F+B∧F∧D//"},
		{'D' ,"|CFB-F+B|FA&F∧A&&FB-F+B|FC//"},
	};
	std::string tree = rewrite_times(2, axiom, transformations);

	while(!WindowShouldClose()) {
		UpdateCamera(&camera, CAMERA_FREE);
		if (IsKeyPressed(KEY_Z))
			camera.target = (Vector3){ 0.0f, 0.0f, 0.0f};

		BeginDrawing(); {
			ClearBackground(RAYWHITE);
            BeginMode3D(camera); {

				t.reset();
				t.follow_string(tree);

                // DrawCube(cubePosition, 2.0f, 2.0f, 2.0f, RED);
                // DrawCubeWires(cubePosition, 2.0f, 2.0f, 2.0f, MAROON);
                // DrawGrid(10, 1.0f);

            } EndMode3D();
		} EndDrawing();
	}
	CloseWindow();
	return 0;
}
