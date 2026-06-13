#include "geom.hpp"

Matrix3::Matrix3(float f00, float f01, float f02,
				 float f10, float f11, float f12,
				 float f20, float f21, float f22)
	:f{{f00, f01, f02},
	   {f10, f11, f12},
	   {f20, f21, f22}} {}

std::array<float, 3> Matrix3::operator*(const std::array<float, 3>& vec) {
	return {
		(vec[0] * f[0][0]) + (vec[1] * f[0][1]) + (vec[2] * f[0][2]),
		(vec[0] * f[1][0]) + (vec[1] * f[1][1]) + (vec[2] * f[1][2]),
		(vec[0] * f[2][0]) + (vec[1] * f[2][1]) + (vec[2] * f[2][2]),
	};
}

Turtle::Turtle(float angle, float stride, float thickness)
	:angle(angle),stride(stride),thickness(thickness){}
void Turtle::follow_string(const std::string& s) {
	for(const char c : s) follow_char(c);
}
void Turtle::reset() {
	state_stack.clear();
	curr = (state) {
		{0, 0, 0},
		{0, 1, 0},
	};
}

void Turtle::log_state() {
	float x = curr.pos.x;
	float y = curr.pos.y;
	float z = curr.pos.z;

	float h = curr.dir.h;
	float l = curr.dir.l;
	float u = curr.dir.u;

	std::cout<< "Pos: {" << x << ", " << y << ", " << z << "}\n"
			 << "Dir: {"<< h << ", " << l << ", " << u << "}"
			 << std::endl;
}

std::array<float, 3> Turtle::rotate_h(const std::array<float, 3>& vec,
									  const float alpha){
	return Matrix3(+cos(alpha), +sin(alpha), 0,
				   -sin(alpha), +cos(alpha), 0,
				   0, 0, 1) * vec;
}
std::array<float, 3> Turtle::rotate_l(const std::array<float, 3>& vec,
									  const float alpha){
	return Matrix3(+cos(alpha), 0, -sin(alpha),
				   0, 1, 0,
				   +sin(alpha), 0, +cos(alpha)) * vec;
}
std::array<float, 3> Turtle::rotate_u(const std::array<float, 3>& vec,
									  const float alpha){
	return Matrix3(1, 0, 0,
				   0, +cos(alpha), -sin(alpha),
				   0, +sin(alpha), +cos(alpha)) * vec;
}

Turtle::state Turtle::state::step() {
	return (Turtle::state) {
		{
			pos.x + dir.h,
			pos.y + dir.u,
			pos.z + dir.l,
		},
		dir,
	};
}

void Turtle::state::rotate_h_by(const float angle) {
	auto new_hlu = rotate_l({dir.h, dir.l, dir.u}, angle);
	dir = {new_hlu[0], new_hlu[1], new_hlu[2]};
}

void Turtle::state::rotate_l_by(const float angle) {
	auto new_hlu = rotate_l({dir.h, dir.l, dir.u}, angle);
	dir = {new_hlu[0], new_hlu[1], new_hlu[2]};
}

void Turtle::state::rotate_u_by(const float angle) {
	auto new_hlu = rotate_u({dir.h, dir.l, dir.u}, angle);
	dir = {new_hlu[0], new_hlu[1], new_hlu[2]};
}

void Turtle::follow_char(const char c) {
	switch(c) {
	case '[':
		state_stack.push_back(curr);
		break;
	case ']':
		curr = state_stack.back();
		state_stack.pop_back();
		break;
	case 'F':
		DrawCube(curr.pos,
				 thickness, thickness, thickness,
				 RED);
		DrawLine3D(curr.pos, curr.step().pos, BLACK);
		curr = curr.step();
		break;
	case 'f':
		curr = curr.step();
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
	default:
		(void)c;
		break;
	}
}
