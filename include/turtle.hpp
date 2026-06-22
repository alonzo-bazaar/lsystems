#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <array>
#include <cmath>

#include "raylib.h"
// internal usati per la funzione DrawTriangleFan3D
#include "rlgl.h"
// par_shapes è la libreria usata da raylib per generare mesh
// visto che l'api di raylib non ci basta per la mesh che vogliamo fare
// usiamo direttamente la libreria interna sua per creare la mesh che poi
// passeremo a raylib
#include "external/par_shapes.h"

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
		   const std::vector<std::array<float, 2>>& texcoords_table);

	Model follow_string(const std::string& s);
	void log_state();

private:
	float angle;
	float stride;
    const std::vector<float> thickness_table;
	const std::vector<std::array<float, 2>> texcoords_table;

    std::vector<Vector3> current_polygon = {};
    bool polygon_mode = false;

	struct State {
		// un'istanza di Turtle::state non è necessariamente
		// legata a un'istanza di Turtle
		// quindi se vogliamo che un Turtle::state abbia accesso ai campi
		// della Turtle di cui è state dobbiamo dargli un puntatore o
		// qualcosa al Turtle di afferenza
		const Turtle* owner;

		// nello stato ci mettiamo gli indici alla color/thickness table
		// invece che metterci il colore o la thickness visto che
		// se ce mettiamo colore o thickness poi avanzare lo stato al colore
		// o thickness successivi come famo?
		// non famo
		// quindi lo stato c'ha gli indici
		size_t thickness_table_index;
		size_t texcoords_table_index;
		Vector3 pos;
        Matrix3 hlu;

		State step();
        // vedi pagina 19 del pdf docs/book/abop-ch1.pdf
		void rotate_u_by(const float angle);
		void rotate_l_by(const float angle);
		void rotate_h_by(const float angle);
	};
	State current_state = (State) {
		this,
		0, 0,
		{0, 0, 0},
		// matrice dove la prima colonna è H / heading
		// la seconda è L / left
		// la terza è U / up
		// se vogliamo che la direzione iniziale della tartaruga sia verso
		// l'alto allora heading/la prima colonna deve essere verso l'alto
		// che per il nostro sistema di riferimento vuol dire verso le y
		// positive.
		// L e U possono essere quello che gli pare, basta che HLU sia una 
		// base ortonormale
		{0, 0, 1,
         1, 0, 0,
         0, 1, 0},
	};
	std::vector<State> state_stack {};
	struct MeshBuilder {
		const Turtle* owner;

		// punti rappresentati come xyzxyzxyz... tutti attaccati
		std::vector<float> points;
		// normali dei varii triangoli, sempre xyzxyz... tutti attaccati
		std::vector<float> normals;
		// coordinate texture rappresntate come uvuvuvuv... attaccati
		std::vector<float> texcoords;

		// i vettori li rappresentiamo così piatti invece di fare un vettore 
		// di struct per poterli passare più facilmente all'api poi di raylib
		// che si aspetta degli array piatti di xyzxyzxyz... o uvuvuvuv...
		// o via dicendo

		// queste funzioni prendono thickness, texcoords, et al da owner
		// (owner->current_texcoords() / owner->current_thickness(), et al.)
		void add_cylinder(Vector3 start, Vector3 end);
		void add_polygon(std::vector<Vector3> points);
		Mesh get();
	};
	MeshBuilder mesh_builder{this, {}, {}, {}};

	void follow_char(const char c);

	// funzioni di convenience visto che scrivere
	// color_table[current_state.color_table_index]
	// ogni volta è un po' una rottura
	const std::array<float, 2> current_texcoords() const;
	const float current_thickness() const;
};
