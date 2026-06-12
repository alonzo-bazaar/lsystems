#include "raylib.h"

int main() {
	InitWindow(640, 480, "Hello Motherfucker");
	while(!WindowShouldClose()) {
		BeginDrawing();

		ClearBackground(RAYWHITE);
		DrawText("English motherfucker do you speak it?",
				 190, 200, 20, LIGHTGRAY);
		EndDrawing();
	}
	CloseWindow();
	return 0;
}
