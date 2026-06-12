#include<vector>
#include<string>
#include<stack>
#include<cmath>
#include<initializer_list>

#include "rewrite.hpp"
#include "raylib.h"

class Matrix3 {
public:
	Matrix3(float f00, float f01, float f02,
			float f10, float f11, float f12,
			float f20, float f21, float f22)
		:f{{f00, f01, f02},
		   {f10, f11, f12},
		   {f20, f21, f22}} {}

	Vector3 operator*(const Vector3& in) {
		return (Vector3) {
			.x = (in.x * f[0][0]) + (in.y * f[0][1]) + (in.z * f[0][2]),
			.y = (in.x * f[1][0]) + (in.y * f[1][1]) + (in.z * f[1][2]),
			.z = (in.x * f[2][0]) + (in.y * f[2][1]) + (in.z * f[2][2]),
		};
	}
private:
	const float f [3][3];
};

// per adesso disegnamo un affare 3d a merda sullo
class Turtle {
public:
	Turtle(float angle, float stride, float thickness)
		:angle(angle),stride(stride),thickness(thickness){}
	void follow_string(const std::string& s) {
		for(const char c : s) follow_char(c);
	}
	void reset() {
		state_stack.clear();
		current_state = (state){
			.position = (Vector3){0, 0, 0},
			.direction = (Vector3){0, 1, 0},
		};
	}

	void log_state() {
		float x = current_state.position.x;
		float y = current_state.position.y;
		float z = current_state.position.z;

		// h l u = heading, left, up
		// h = x
		// l = z
		// u = y
		// ho sminchiato l'interpretazione del sistema di
		// coordinate dato nel paper
		// per dindirindina (diocane)
		float h = current_state.position.x;
		float l = current_state.position.y;
		float u = current_state.position.z;

		std::cout<<
			"Position :" << 
	}

private:
	float angle;
	float stride;
	float thickness;

	struct state {
		Vector3 position;
		Vector3 direction;
	};

	state current_state = (state){
		.position = (Vector3){0, 0, 0},
		.direction = (Vector3){0, 1, 0},
	};

	// vedi pagina 19 del pdf docs/book/abop-ch1.pdf
	Vector3 rotate_u(Vector3 vec, float alpha) {
		return Matrix3(+cos(alpha), +sin(alpha), 0,
					   -sin(alpha), +cos(alpha), 0,
					   0, 0, 1) * vec;
	}
	Vector3 rotate_l(Vector3 vec, float alpha) {
		return Matrix3(+cos(alpha), 0, -sin(alpha),
					   0, 1, 0,
					   +sin(alpha), 0, +cos(alpha)) * vec;
	}
	Vector3 rotate_h(Vector3 vec, float alpha) {
		return Matrix3(1, 0, 0,
					   0, +cos(alpha), -sin(alpha),
					   0, +sin(alpha), +cos(alpha)) * vec;
	}

	std::vector<state> state_stack;
	void follow_char(const char c) {
		switch(c) {
		case '[':
			state_stack.push_back(current_state);
			break;
		case ']':
			current_state = state_stack.back();
			state_stack.pop_back();
			break;
		case 'F':
			DrawCube(current_state.position,
					 thickness, thickness, thickness,
					 RED);
			current_state.position.x += current_state.direction.x;
			current_state.position.y += current_state.direction.y;
			current_state.position.z += current_state.direction.z;
			(void)0;
			break;
		case 'f':
			current_state.position.x += current_state.direction.x;
			current_state.position.y += current_state.direction.y;
			current_state.position.z += current_state.direction.z;
			(void)0;
			break;

		case '+':
			current_state.direction = rotate_u(current_state.direction,
											   angle);
			break;
		case '-':
			current_state.direction = rotate_u(current_state.direction,
											   -angle);
			break;
		case '&':
			current_state.direction = rotate_l(current_state.direction,
											   angle);
			break;
		case '^':
			current_state.direction = rotate_l(current_state.direction,
											   -angle);
			break;
		case '\\':
			current_state.direction = rotate_h(current_state.direction,
											   angle);
			break;
		case '/':
			current_state.direction = rotate_h(current_state.direction,
											   -angle);
			break;

		case '|':
			current_state.direction = rotate_u(current_state.direction,
											   PI);
			break;

		default:
			break;
		}
	}
};

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
