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
	int target_fps = 60;
	SetTargetFPS(target_fps);

	Camera camera = { 0 };
    camera.position = (Vector3){ 10.0f, 10.0f, 10.0f };
    camera.target = (Vector3){ 0.0f, 0.0f, 0.0f };
    camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };
    camera.fovy = 45.0f;
    camera.projection = CAMERA_PERSPECTIVE;



	// n=2, δ=90◦
	std::string hilbert_axiom = "A";
	std::map<char, std::string> hilbert_trans {
		{'A' ,"B-F+CFC+F-D&F^D-F+&&CFC+F+B//"},
		{'B' ,"A&F^CFB^F^D^^-F-D^|F^B|FC^F^A//"},
		{'C' ,"|D^|F^B-F+C^F^A&&FA&F^C+F+B^F^D//"},
		{'D' ,"|CFB-F+B|FA&F^A&&FB-F+B|FC//"},
	};
	std::string hilbert = rewrite_times(2, hilbert_axiom, hilbert_trans);
	// Turtle t(PI/2, 1.0f, {0.1f}, {LIGHTGRAY});
	// std::string& target = hilbert;

	// n=7, δ=22.5◦
	std::string tree_axiom = "A";
	std::map<char, std::string> tree_trans = {
		{'A', "[&FL!A]/////'[&FL!A]///////'[&FL!A]"},
		{'F', "S ///// F"},
		{'S', "F L"},
		{'L', "['''^^{-f+f+f-|-f+f+f}]"},
	};
	std::string tree = rewrite_times(7, tree_axiom, tree_trans);
	Turtle t(deg_to_rad(22.5), 0.30f,
			 map_range<float>(0.06f, 0.015f, 7),
			 map_range<Color>(BROWN, GREEN, 7));
	std::string& target = tree;

	while(!WindowShouldClose()) {
		UpdateCamera(&camera, CAMERA_FIRST_PERSON);
		if (IsKeyPressed(KEY_Z))
			camera.target = (Vector3){ 0.0f, 0.0f, 0.0f};

		BeginDrawing(); {
			ClearBackground(RAYWHITE);
            BeginMode3D(camera); {
				t.reset();
				t.follow_string(target);
            } EndMode3D();
			DrawText(TextFormat("FPS: %i (target %i)", GetFPS(), target_fps),
					 10, 10, 20, DARKGRAY);
		} EndDrawing();
	}
	CloseWindow();
	return 0;
}
