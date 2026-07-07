#pragma once
#ifndef LSYSTEMS_TURTLE_HPP_
#define LSYSTEMS_TURTLE_HPP_

#include <iostream>
#include <string>
#include <vector>
#include <array>
#include <utility>
#include <cmath>

#include "raylib.h"
// internal usati per la funzione DrawTriangleFan3D
#include "rlgl.h"
// par_shapes è la libreria usata da raylib per generare mesh
// visto che l'api di raylib non ci basta per la mesh che vogliamo fare
// usiamo direttamente la libreria interna sua per creare la mesh che poi
// passeremo a raylib
#include "external/par_shapes.h"
// per instruction
#include "rewrite.hpp"
#include "player.hpp"

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

struct TreeModel {
    const BoundingBox bounding_box;
    const Model model;

    TreeModel(Model m):
        model(m),
        bounding_box(GetModelBoundingBox(m)) {}

    static BoundingBox bb_shift(const BoundingBox bb, const Vector3 p) {
        return {Vector3Add(bb.min, p), Vector3Add(bb.max, p)};
    }

    void draw(const Vector3 pos) const {
        DrawModel(model, pos, 1.0f, WHITE);
    }

    void draw(const Vector3 pos, const Player& player) const {
        if(player.can_see(bounding_box))
            draw(pos);
    }
};

class Turtle {
public:
	Turtle(const std::vector<float>& thickness_table,
		   const std::vector<std::array<float, 2>>& texcoords_table);

	TreeModel follow_instruction_vector(const std::vector<instruction>& iv);
	void log_state();

private:
    const std::vector<float> thickness_table;
	const std::vector<std::array<float, 2>> texcoords_table;

    std::vector<Vector3> current_polygon = {};
    bool polygon_mode = false;

	struct State {
		// nello stato ci mettiamo gli indici alla color/thickness table
		// invece che metterci il colore o la thickness visto che vogliamo che
		// un'istanza di state abbia tutte le informazioni per calcolare lo
		// state successivo a una trasformazione quale, ad esempio
		// passare al colore (o spessore) successivo
		size_t thickness_table_index;
		size_t texcoords_table_index;
		Vector3 pos;
        Matrix3 hlu;

		State step_by(const float step_length);
        // vedi pagina 19 del pdf docs/book/abop-ch1.pdf per come
        // sono implementate queste 3 funzioni
		void rotate_u_by(const float angle);
		void rotate_l_by(const float angle);
		void rotate_h_by(const float angle);
	};
	State current_state {
		0, 0,
		{0, 0, 0},
		// matrice HLU per indicare l'orientamento della tartaruga
		// nello spazio:
		// la prima colonna è H / heading
		// la seconda è L / left
		// la terza è U / up
		// se vogliamo che la direzione iniziale della tartaruga sia verso
		// l'alto allora heading/la prima colonna deve essere verso l'alto
		// che per il nostro sistema di riferimento vuol dire verso le y
		// positive.
		// L e U possono essere messe a piacimento, basta che HLU formi una
		// base ortonormale
		{0, 0, 1,
         1, 0, 0,
         0, 1, 0}};

	std::vector<State> state_stack {};
	struct MeshBuilder {
		// un'istanza di Turtle::mesh_builder non è necessariamente
		// legata a un'istanza "genitore" di Turtle
		// quindi se vogliamo che un Turtle::mesh_builder abbia accesso ai
		// campi della Turtle di cui è state, dobbiamo dargli un puntatore o
		// qualcosa al Turtle di afferenza
		const Turtle* owner;

		// punti rappresentati come xyzxyzxyz... tutti attaccati
		std::vector<float> points;
		// normali dei varii triangoli, sempre xyzxyz... tutti attaccati
		std::vector<float> normals;
		// coordinate texture rappresntate come uvuvuvuv... attaccati
		std::vector<float> texcoords;

		// i vettori li rappresentiamo così piatti invece di fare un vettore 
		// di struct per poterli passare più facilmente all'api poi di raylib
		// che, per passarli a opengli, si aspetta degli array piatti
		// xyzxyzxyz... o uvuvuvuv... o via dicendo

		// queste funzioni prendono thickness, texcoords, et al da owner
		// non vi è quindi bisogno di passargli ulteriori informazioni su
		// come va fatto il cilindro/poligono
		void add_cylinder(Vector3 start, Vector3 end);
		void add_polygon(std::vector<Vector3> points);
		Mesh get();
	};
	MeshBuilder mesh_builder;

	void follow_instruction(const instruction& i);

	// funzioni di convenience visto che scrivere
	const std::array<float, 2> current_texcoords() const;
	const float current_thickness() const;
};


#endif // LSYSTEMS_TURTLE_HPP_
