#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <array>
#include <cmath>

#include "raylib.h"
#include "rlgl.h" // to draw triangle fan

void DrawTriangleFan3D(const Vector3* points, int point_count, Color color);

class Matrix3 {
public:
	Matrix3(float f00, float f01, float f02,
			float f10, float f11, float f12,
			float f20, float f21, float f22);
    Matrix3(const Matrix3& mat);
    // https://en.cppreference.com/cpp/language/copy_assignment
    const Matrix3& operator=(const Matrix3& rhs);

	std::array<float, 3> col(const unsigned int n) const;
	std::array<float, 3> row(const unsigned int n) const;

    // per accedere
    float* operator[](const unsigned int r);
    std::array<float, 3> operator()(const unsigned int r) const;
    float operator()(const unsigned int r,
                     const unsigned int c) const;

    static Matrix3 from_rows(const std::array<float, 3>& r0,
                             const std::array<float, 3>& r1,
                             const std::array<float, 3>& r2);
    static Matrix3 from_cols(const std::array<float, 3>& c0,
                             const std::array<float, 3>& c1,
                             const std::array<float, 3>& c2);
    Matrix3 trans() const;

    // product with column vector
	std::array<float, 3> operator*(const std::array<float, 3>& vec) const;
    // product with other 3x3 matrix
	Matrix3 operator*(const Matrix3& rhs) const;
private:
	float f[3][3];
};

class Turtle {
public:
	Turtle(float angle,
           float stride,
           const std::vector<float>& thickness_table,
           const std::vector<Color>& color_table);

	void follow_string(const std::string& s);
	void reset();
	void log_state();

private:
	float angle;
	float stride;
    std::vector<float> thickness_table;
    std::vector<Color> color_table;

    std::vector<Vector3> current_polygon = {};
    bool polygon_mode = false;

	struct state {
		// un'istanza di Turtle::state non è necessariamente
		// legata a un'istanza di Turtle
		// quindi se vogliamo che un Turtle::state abbia accesso ai campi
		// della Turtle di cui è state dobbiamo dargli un puntatore o
		// qualcosa al Turtle di afferenza
		Turtle* owner;
		int thickness_table_index;
		int color_table_index;
		Vector3 pos;
        Matrix3 hlu;

		state step();
        // vedi pagina 19 del pdf docs/book/abop-ch1.pdf
		void rotate_u_by(const float angle);
		void rotate_l_by(const float angle);
		void rotate_h_by(const float angle);
	};
	state curr = (state) {
		this,
		0,
		0,
		{0, 0, 0},
		{0, 0, 1,
         1, 0, 0,
         0, 1, 0},
	};
	std::vector<state> state_stack { curr };
	void follow_char(const char c);

	static int clamp(int a, int from, int to);
	Color curr_color;
	float curr_thickness;
	void update_curr_color();
	void update_curr_thickness();
};
