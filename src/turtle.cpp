#include "turtle.hpp"

// utility function to draw leaves
// should probably put it somewhere other than here
void DrawTriangleFan3D(const Vector3* points, int point_count, Color color) {
	rlBegin(RL_TRIANGLES); {
		rlColor4ub(color.r, color.g, color.b, color.a);
		int i = 1;
		while(i) {
			int j = (i+1)%point_count;

			rlVertex3f(points[0].x, points[0].y, points[0].z);
			rlVertex3f(points[i].x, points[i].y, points[i].z);
			rlVertex3f(points[j].x, points[j].y, points[j].z);
			// both sides
			rlVertex3f(points[0].x, points[0].y, points[0].z);
			rlVertex3f(points[j].x, points[j].y, points[j].z);
			rlVertex3f(points[i].x, points[i].y, points[i].z);

			i=j;
		}
	} rlEnd();
}

Matrix3::Matrix3(float f00, float f01, float f02,
				 float f10, float f11, float f12,
				 float f20, float f21, float f22)
	:f{{f00, f01, f02},
	   {f10, f11, f12},
	   {f20, f21, f22}} {}

Matrix3::Matrix3(const Matrix3& mat)
    :f{{mat(0, 0), mat(0, 1), mat(0, 2)},
	   {mat(1, 0), mat(1, 1), mat(1, 2)},
	   {mat(2, 0), mat(2, 1), mat(2, 2)}} {}

const Matrix3& Matrix3::operator=(const Matrix3&rhs) {
    f[0][0] = rhs(0, 0); f[0][1] = rhs(0, 1); f[0][2] = rhs(0, 2);
    f[1][0] = rhs(1, 0); f[1][1] = rhs(1, 1); f[1][2] = rhs(1, 2);
    f[2][0] = rhs(2, 0); f[2][1] = rhs(2, 1); f[2][2] = rhs(2, 2);
    return *this;
}

Matrix3 Matrix3::from_rows(const std::array<float, 3>& r0,
                           const std::array<float, 3>& r1,
                           const std::array<float, 3>& r2) {
    return Matrix3(r0[0], r0[1], r0[2],
                   r1[0], r1[1], r1[2],
                   r2[0], r2[1], r2[2]);
}

Matrix3 Matrix3::from_cols(const std::array<float, 3>& c0,
                           const std::array<float, 3>& c1,
                           const std::array<float, 3>& c2) {
    return {c0[0], c1[0], c2[0],
            c0[1], c1[1], c2[1],
            c0[2], c1[2], c2[2],};
}

std::array<float, 3> Matrix3::col(const unsigned int n) const {
    return {f[0][n], f[1][n], f[2][n]};
}

std::array<float, 3> Matrix3::row(const unsigned int n) const {
    return {f[n][0], f[n][1], f[n][2]};
}

// ho fatto un attimo sta cazzata che [] è per accedere e mutare gli elementi
// della matrice mentre () è per ricevere una copia degli elementi
// ha senso? no
// funziona? più del dovuto
float* Matrix3::operator[](const unsigned int r) {
    return f[r];
}
std::array<float, 3> Matrix3::operator()(const unsigned int r) const {
    return row(r);
}
float Matrix3::operator()(const unsigned int r,
                          const unsigned int c) const {
    return f[r][c];
}


Matrix3 Matrix3::trans() const {
    return from_cols(row(0), row(1), row(2));
}

// moltiplica per un vettore colonna
// (il vettore colonna lo passo come riga per semplificarmi la vita)
std::array<float, 3> Matrix3::operator*(const std::array<float, 3>& vec) const {
	return {
		(vec[0] * f[0][0]) + (vec[1] * f[0][1]) + (vec[2] * f[0][2]),
		(vec[0] * f[1][0]) + (vec[1] * f[1][1]) + (vec[2] * f[1][2]),
		(vec[0] * f[2][0]) + (vec[1] * f[2][1]) + (vec[2] * f[2][2]),
	};
}

// moltiplica per un'altra matrice 3x3
Matrix3 Matrix3::operator*(const Matrix3& rhs) const {
    return from_cols((*this)*(rhs.col(0)),
                     (*this)*(rhs.col(1)),
                     (*this)*(rhs.col(2)));
}


Turtle::Turtle(float angle,
               float stride,
			   const std::vector<float>& thickness_table,
               const std::vector<Color>& color_table)
	:angle(angle),
     stride(stride),
     thickness_table(thickness_table),
     color_table(color_table) {}

void Turtle::follow_string(const std::string& s) {
	for(const char c : s) follow_char(c);
}

void Turtle::reset(Vector3 pos) {
	state_stack.clear();
	curr = (state) {
		this,
		0,
		0,
		pos,
		// in coordinate xyz
		// l'heading iniziale (prima colonna) vogliamo sia verso l'alto
		// quindi y
		// l'up e il left iniziale... boh, vanno bene entrambi penso
		{0, 0, 1,
         1, 0, 0,
         0, 1, 0},
	};
}

void Turtle::log_state() {
	auto x = curr.pos.x;
	auto y = curr.pos.y;
	auto z = curr.pos.z;

	auto h = curr.hlu.col(0);
    auto l = curr.hlu.col(1);
    auto u = curr.hlu.col(2);

	std::cout<< "Pos: {" << x << ", " << y << ", " << z << "}\n"
			 << "Dir: {\n"
             << "  h = {" << h[0] << ", " << h[1] << ", " << h[2] << "},\n"
             << "  l = {" << l[0] << ", " << l[1] << ", " << l[2] << "},\n"
             << "  u = {" << u[0] << ", " << u[1] << ", " << u[2] << "},\n"
             << "}"
             << std::endl;
}

Turtle::state Turtle::state::step() {
    // vai in direzione heading di un tot
    // forse dovrei separare posizione e metadati, sta funzione mi pare
    // un po' stronza mo'
    auto h = hlu.col(0);
	return (Turtle::state) {
		owner,
		thickness_table_index,
		color_table_index,
		{
			pos.x + (h[0] * owner->stride),
			pos.y + (h[1] * owner->stride),
			pos.z + (h[2] * owner->stride),
		},
		hlu,
	};
}

void Turtle::state::rotate_h_by(const float alpha) {
    hlu = hlu * Matrix3(1, 0, 0,
                        0, +cos(alpha), -sin(alpha),
                        0, +sin(alpha), +cos(alpha));
}

void Turtle::state::rotate_l_by(const float alpha) {
    hlu = hlu * Matrix3(+cos(alpha), 0, -sin(alpha),
                        0, 1, 0,
                        +sin(alpha), 0, +cos(alpha));
}

void Turtle::state::rotate_u_by(const float alpha) {
    hlu = hlu * Matrix3(+cos(alpha), +sin(alpha), 0,
                        -sin(alpha), +cos(alpha), 0,
                        0, 0, 1);
}


void Turtle::follow_char(const char c) {
	switch(c) {
	case '[':
		state_stack.push_back(curr);
		break;
	case ']':
		curr = state_stack.back();
		update_curr_color();
		update_curr_thickness();
		state_stack.pop_back();
		break;

	case '{':
		polygon_mode = true;
        current_polygon.push_back(curr.pos);
		break;
	case '}':
		polygon_mode = false;
        // disegna poligono
        // per farlo disengamo i triangloli
        // 0 1 2
        // 0 2 3
        // 0 3 4
        // ...
        // 0 n 1
        // visto che le foglie sono 3d vogliamo che siano visibili da entrambi
        // i lato, disegnamo i triangoli da entrambe le direzioni
        // (poi si vede dallo shader come fare che la direzione su sia più
        //  chiara e quella giù più scura)
		DrawTriangleFan3D(current_polygon.data(), current_polygon.size(),
						 curr_color);
		current_polygon.clear();
		break;

	case 'F': {
        auto old_pos = curr.pos;
        curr = curr.step();
		DrawCylinderEx(old_pos, curr.pos,
					   curr_thickness, curr_thickness, 4, curr_color);
		break;
    }
	case 'f':
		curr = curr.step();
        if(polygon_mode)
            current_polygon.push_back(curr.pos);
		break;

	case '\\':
		curr.rotate_h_by(angle);
		break;
	case '/':
		curr.rotate_h_by(-angle);
		break;

	case '&':
		curr.rotate_l_by(angle);
		break;
	case '^':
		curr.rotate_l_by(-angle);
		break;

	case '+':
		curr.rotate_u_by(angle);
		break;
	case '-':
		curr.rotate_u_by(-angle);
		break;
	case '|':
		curr.rotate_u_by(PI);
		break;

	case '!':
		curr.thickness_table_index++;
		update_curr_thickness();
		break;
	case '\'':
		curr.color_table_index++;
		update_curr_color();
		break;
	default:
		// std::cout<<"ignoring: '"<<c<<'\''<<std::endl;
		(void)c;
		break;
	}
}

int Turtle::clamp(int n, int from, int to) {
	if(n <= from) return from;
	if(n >= to) return to;
	return n;
}

void Turtle::update_curr_color() {
	curr_color = color_table[clamp(curr.color_table_index,
								   0,
								   color_table.size()-1)];
}

void Turtle::update_curr_thickness() {
	curr_thickness = thickness_table[clamp(curr.thickness_table_index,
										   0,
										   thickness_table.size()-1)];
}
