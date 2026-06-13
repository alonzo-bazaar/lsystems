#include<vector>
#include<string>
#include<stack>
#include<cmath>
#include<initializer_list>
#include <iostream>

#include "rewrite.hpp"
#include "raylib.h"
#include "raymath.h"

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
       : angle(angle), stride(stride), thickness(thickness) {
        reset();
    }

    void follow_string(const std::string& s) {
       for(const char c : s) follow_char(c);
    }

    void reset() {
       state_stack.clear();
       // Stato iniziale standard per ABOP:
       // H guarda verso l'alto (0, 1, 0), L guarda a sinistra (-1, 0, 0), U guarda in avanti (0, 0, 1)
       // Oppure puoi usare la configurazione classica di Raylib:
       current_state = (state){
          .position = (Vector3){0, 0, 0},
          .H = (Vector3){0, 1, 0},  // Heading (direzione di movimento)
          .L = (Vector3){-1, 0, 0}, // Left
          .U = (Vector3){0, 0, 1}   // Up
       };
    }

private:
    float angle;
    float stride;
    float thickness;

    struct state {
       Vector3 position;
       Vector3 H; // Heading
       Vector3 L; // Left
       Vector3 U; // Up
    };

    state current_state;
    std::vector<state> state_stack;

    // Ruota i vettori attorno all'asse U (imbardata / yaw) di un angolo alpha
    void rotate_u(float alpha) {
        current_state.H = Vector3RotateByAxisAngle(current_state.H, current_state.U, alpha);
        current_state.L = Vector3RotateByAxisAngle(current_state.L, current_state.U, alpha);
    }

    // Ruota i vettori attorno all'asse L (beccheggio / pitch) di un angolo alpha
    void rotate_l(float alpha) {
        current_state.H = Vector3RotateByAxisAngle(current_state.H, current_state.L, alpha);
        current_state.U = Vector3RotateByAxisAngle(current_state.U, current_state.L, alpha);
    }

    // Ruota i vettori attorno all'asse H (rollio / roll) di un angolo alpha
    void rotate_h(float alpha) {
        current_state.L = Vector3RotateByAxisAngle(current_state.L, current_state.H, alpha);
        current_state.U = Vector3RotateByAxisAngle(current_state.U, current_state.H, alpha);
    }

    void follow_char(const char c) {
       switch(c) {
       case '[':
          state_stack.push_back(current_state);
          break;
       case ']':
          if (!state_stack.empty()) {
              current_state = state_stack.back();
              state_stack.pop_back();
          }
          break;
       	case 'F': {
       		// 1. Calcoliamo dove finirà la tartaruga dopo questo passo
       		Vector3 next_position = Vector3Add(current_state.position, Vector3Scale(current_state.H, stride));

       		// 2. Disegniamo un cilindro che unisce la posizione attuale a quella successiva
       		float radius = thickness / 2.0f;
       		DrawCylinderEx(
				   current_state.position, // Inizio del ramo
				   next_position,          // Fine del ramo
				   radius,                 // Raggio alla base
				   radius * 0.5f,                 // Raggio in cima (usa un valore minore se vuoi un ramo che si rimpicciolisce)
				   8,                      // Lati del cilindro (più è alto, più è l'effetto è liscio)
				   GREEN                   // Colore
			   );

       		// 3. Aggiorniamo la posizione della tartaruga facendola avanzare
       		current_state.position = next_position;
       		break;
       	}

       case 'f':
          // Spostamento senza disegnare
          current_state.position = Vector3Add(current_state.position, Vector3Scale(current_state.H, stride));
          break;

       // Rotazioni attorno all'asse U (Turn Left / Right)
       case '+':
          rotate_u(angle);
          break;
       case '-':
          rotate_u(-angle);
          break;

       // Rotazioni attorno all'asse L (Pitch Down / Up)
       case '&':
          rotate_l(angle);
          break;
       case '^':
          rotate_l(-angle);
          break;

       // Rotazioni attorno all'asse H (Roll Left / Right)
       case '\\':
          rotate_h(angle);
          break;
       case '/':
          rotate_h(-angle);
          break;

       // Ruota di 180 gradi (indietro)
       case '|':
          rotate_u(PI);
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

	InitWindow(640*2, 480*2, "Hello World");
	Camera camera = { 0 };
    camera.position = (Vector3){ 10.0f, 0.0f, 0.0f };
    camera.target = (Vector3){ 0.0f, 0.0f, 0.0f };
    camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };
    camera.fovy = 45.0f;
    camera.projection = CAMERA_PERSPECTIVE;

    // Vector3 cubePosition = { 0.0f, 0.0f, 0.0f };

	// Model cube = LoadModelFromMesh(GenMeshCube(2.0f, 2.0f, 2.0f));

	Turtle t(PI/2, 0.1f, 0.05f);

	// n=2, δ=90◦
	std::string axiom = "A";
	std::map<char, std::string> transformations {
		{'A' ,"B-F+CFC+F-D&F∧D-F+&&CFC+F+B//"},
		{'B' ,"A&F∧CFB∧F∧D∧∧-F-D∧|F∧B|FC∧F∧A//"},
		{'C' ,"|D∧|F∧B-F+C∧F∧A&&FA&F∧C+F+B∧F∧D//"},
		{'D' ,"|CFB-F+B|FA&F∧A&&FB-F+B|FC//"},
	};
	std::string tree = rewrite_times(3, axiom, transformations);

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
