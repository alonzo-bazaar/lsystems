#ifndef LSYSTEMS_TURTLE_H_
#define LSYSTEMS_TURTLE_H_

#include <iostream>
#include <string>
#include <vector>
#include <array>
#include <cmath>

#include "raylib.h"

class Matrix3 {
public:
	Matrix3(float f00, float f01, float f02,
			float f10, float f11, float f12,
			float f20, float f21, float f22);
	std::array<float, 3> operator*(const std::array<float, 3>& vec);
private:
	const float f[3][3];
};

class Turtle {
public:
	Turtle(float angle, float stride, float thickness);
	void follow_string(const std::string& s);
	void reset();
	void log_state();

private:
	float angle;
	float stride;
	float thickness;

	// vedi pagina 19 del pdf docs/book/abop-ch1.pdf
	static std::array<float, 3> rotate_u(const std::array<float, 3>& vec,
										 const float alpha);
	static std::array<float, 3> rotate_l(const std::array<float, 3>& vec,
										 const float alpha);
	static std::array<float, 3> rotate_h(const std::array<float, 3>& vec,
										 const float alpha);

	struct state {
		Vector3 pos;
		struct {
			float h;
			float l;
			float u;
		} dir;
		state step();
		void rotate_u_by(const float angle);
		void rotate_l_by(const float angle);
		void rotate_h_by(const float angle);
	};
	state curr = (state) {
		{0, 0, 0},
		{0, 1, 0},
	};
	std::vector<state> state_stack { curr };
	void follow_char(const char c);
};

#endif // LSYSTEMS_TURTLE_H_
