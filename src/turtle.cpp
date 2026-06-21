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

Model Turtle::follow_string(const std::string& s) {
    for(const char c : s)
		// follow_char() modifica mesh_builder
		// (la tartaruga aggiunge poligoni a mesh_builder man mano che
		//  esegue le istruzioni codificate dai vari caratteri)
		follow_char(c);

	// una volta finito di creare la mesh la usiamo per creare l'albero
	Model tree = LoadModelFromMesh(mesh_builder.get());
	// TODO:
	// per adesso hardcodeato, vediamo poi parametrizzarlo
	Image tex_im = GenImageGradientLinear(1, 10, 0, DARKBROWN, GREEN);
	Texture tex_tex = LoadTextureFromImage(tex_im);
	tree.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = tex_tex;
	UnloadImage(tex_im);
	// FIXME: tex_tex leaks (we don't free it when deleting the tree)
	return tree;
}

void Turtle::log_state() {
    auto x = current_state.pos.x;
    auto y = current_state.pos.y;
    auto z = current_state.pos.z;

    auto h = current_state.hlu.col(0);
    auto l = current_state.hlu.col(1);
    auto u = current_state.hlu.col(2);

    std::cout<< "Pos: {" << x << ", " << y << ", " << z << "}\n"
			 << "Dir: {\n"
             << "  h = {" << h[0] << ", " << h[1] << ", " << h[2] << "},\n"
             << "  l = {" << l[0] << ", " << l[1] << ", " << l[2] << "},\n"
             << "  u = {" << u[0] << ", " << u[1] << ", " << u[2] << "},\n"
             << "}"
             << std::endl;
}

Turtle::State Turtle::State::step() {
    // vai in direzione heading di un tot
    // forse dovrei separare posizione e metadati, sta funzione mi pare
    // un po' stronza mo'
    auto h = hlu.col(0);
    return (Turtle::State) {
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

void Turtle::State::rotate_h_by(const float alpha) {
    hlu = hlu * Matrix3(1, 0, 0,
                        0, +cos(alpha), -sin(alpha),
                        0, +sin(alpha), +cos(alpha));
}

void Turtle::State::rotate_l_by(const float alpha) {
    hlu = hlu * Matrix3(+cos(alpha), 0, -sin(alpha),
                        0, 1, 0,
                        +sin(alpha), 0, +cos(alpha));
}

void Turtle::State::rotate_u_by(const float alpha) {
    hlu = hlu * Matrix3(+cos(alpha), +sin(alpha), 0,
                        -sin(alpha), +cos(alpha), 0,
                        0, 0, 1);
}


void Turtle::follow_char(const char c) {
    switch(c) {
    case '[':
		state_stack.push_back(current_state);
		break;
    case ']':
		current_state = state_stack.back();
		state_stack.pop_back();
		break;

    case '{':
		polygon_mode = true;
        current_polygon.push_back(current_state.pos);
		break;
    case '}':
		polygon_mode = false;
		mesh_builder.add_polygon(current_polygon);
		current_polygon.clear();
		break;

    case 'F': {
        auto old_pos = current_state.pos;
        current_state = current_state.step();
		mesh_builder.add_cylinder(old_pos, current_state.pos);
		break;
    }
    case 'f':
		current_state = current_state.step();
		if(polygon_mode)
			current_polygon.push_back(current_state.pos);
		break;

    case '\\':
		current_state.rotate_h_by(angle);
		break;
    case '/':
		current_state.rotate_h_by(-angle);
		break;

    case '&':
		current_state.rotate_l_by(angle);
		break;
    case '^':
		current_state.rotate_l_by(-angle);
		break;

    case '+':
		current_state.rotate_u_by(angle);
		break;
    case '-':
		current_state.rotate_u_by(-angle);
		break;
    case '|':
		current_state.rotate_u_by(PI);
		break;

    case '!':
		if(current_state.thickness_table_index+1 < thickness_table.size())
			current_state.thickness_table_index++;
		break;
    case '\'':
		if(current_state.color_table_index+1 < color_table.size())
			current_state.color_table_index++;
		break;
    default:
		// std::cout<<"ignoring: '"<<c<<'\''<<std::endl;
		(void)c;
		break;
    }
}

const inline Color Turtle::current_color() const {
	return color_table[current_state.color_table_index];
}

const inline float Turtle::current_thickness() const {
	return thickness_table[current_state.thickness_table_index];
}

// funzioni di utility visto che non posso aggiungere metodi a Vector3
// somma
static inline Vector3 add(const Vector3& a, const Vector3& b) {
	return {a.x + b.x, a.y + b.y, a.z + b.z};
}
static inline Vector3 add(const Vector3& a, const float b) {
	return {a.x + b, a.y + b, a.z + b};
}
static inline Vector3 add(const float a, const Vector3& b) {
	return {a + b.x, a + b.y, a + b.z};
}

// sottrazione
static inline Vector3 sub(const Vector3& a, const Vector3& b) {
	return {a.x - b.x, a.y - b.y, a.z - b.z};
}
static inline Vector3 sub(const Vector3& a, const float b) {
	return {a.x - b, a.y - b, a.z - b};
}
static inline Vector3 sub(const float a, const Vector3& b) {
	return {a - b.x, a - b.y, a - b.z};
}

// prodotto per uno scalare
static inline Vector3 mul(const Vector3& a, const float b) {
	return {a.x * b, a.y * b, a.z * b};
}
static inline Vector3 mul(const float a, const Vector3& b) {
	return {a * b.x, a * b.y, a * b.z};
}

// divisione per uno scalare
static inline Vector3 div(const Vector3& a, const float b) {
	return {a.x / b, a.y / b, a.z / b};
}
static inline Vector3 div(const float a, const Vector3& b) {
	return {a / b.x, a / b.y, a / b.z};
}

// norma e normalizzazione
static inline float norm2(const Vector3& v) {
	return sqrt(v.x*v.x + v.y*v.y + v.z*v.z);
}

static inline Vector3 normalized(const Vector3& v) {
	return div(v, norm2(v));
}

void Turtle::MeshBuilder::add_cylinder(Vector3 startpos, Vector3 endpos) {
	// non voglio scassarmi troppo a parametrizzare la qualsiasi quindi
	// queste le fissiamo
	static const unsigned int slices = 4;
	static const unsigned int stacks = 1;

	// inizia creando una par_shapes_mesh contenente il cilindro che vogliamo
	// aggiungere a mesh builder 
	// poi aggiungiamo i punti della mesh in questione al builder
	// poi eliminiamo la mesh visto che quello che ce dovevamo fare s'è fatto

	// crea mesh (inizialmente centrata all'origine
	// dalla documentazione di par_shapes, il cilindro inizialemnte
	// si trova appoggiato all'asse z (ha una base sul piano z=0) e si
    // sviluppa lungo l'asse z in positivo
	par_shapes_mesh *cylinder = par_shapes_create_cylinder(slices, stacks);

	// prima di fare lo scaling e traslazione, par_shapes_create_cylinder
	// non ti aggiunge i "tappi" al cilindro, quindi vanno aggiunti noi
	par_shapes_mesh* cap_top = par_shapes_create_disk
		(1.0f, slices,
		 (float[]){0.0, 0.0, 1.0},  // centro del disco
		 (float[]){0.0, 0.0, 1.0}); // normale del disco
	// par_shapes_create_disk non crea/alloca spazio per le texture coordinate
	// quindi se vogliamo settarele (qui mettiamo a 0) ci s'ha da fa noi tutto
	cap_top->tcoords = PAR_MALLOC(float, 2*cap_top->npoints);
	for (size_t i = 0; i < 2*cap_top->npoints; ++i)
		cap_top->tcoords[i] = 0.0f;

	// fatto  il tappo sopra, idem con patate per il tappo sotto
	par_shapes_mesh* cap_bot = par_shapes_create_disk
		(1.0f, slices,
		 (float[]){0.0, 0.0, 0.0},
		 (float[]){0.0, 0.0, -1.0});
	cap_bot->tcoords = PAR_MALLOC(float, 2*cap_bot->npoints);
	for (size_t i = 0; i < 2*cap_bot->npoints; ++i)
		cap_bot->tcoords[i] = 0.95f;
	// non so per quale motivo il disco, anche se con la normale giusta a -1,
	// lo crea comunque nel verso oppposto da quello che si aspetta raylib
	// per ovviare a questa cosa se flippa il winding del disco
	par_shapes_invert(cap_bot, 0, 0);

	par_shapes_merge_and_free(cylinder, cap_top);
	par_shapes_merge_and_free(cylinder, cap_bot);

	// allunga alla lunghezza desiderata e setta larghezza
	const Vector3 diff = sub(startpos, endpos);
	const float diff_norm = norm2(diff);

	par_shapes_scale(cylinder,
					 owner->current_thickness(), // width  (x)
					 owner->current_thickness(), // height (y)
					 diff_norm);                 // length (z)

	// ruota in modo che orientata lungo la differenza tra startpos e endpos
	// per farlo lo ruotiamo di 180 gradi lungo la bisettrice
	// tra i due vettori, z da una parte, e diff dall'altra
	// per facilitare il calcolo della bisettrice normalizziamo prima diff e
	// famo la media con z, la media tra due versori giace sulla bisettrice
	// tra i due quindi ez
	const Vector3 bisec = normalized(add(normalized(diff), {0, 0, 1}));
	par_shapes_rotate(cylinder, PI, (float[]){bisec.x, bisec.y, bisec.z});

	// ora che ce l'abbiamo alla lunghezza e direzione desiderata
	// lo trasliamo alla posizione desiderata
	par_shapes_translate(cylinder, startpos.x, startpos.y, startpos.z);

	// creato il cilindro aggiungiamo tutti i punti dell suddetto al builder
	// e (motivo per cui ho fatto sto builder) aggiungiamo i corrispettivi
	// colori al builder
	// (vedi GenMeshCylinder in thirdparty/src/rmodels.c per da dove ho preso
	//  il codice per iterare una par_shapes_mesh)

	// par_shapes_mesh è definita come
	// typedef struct par_shapes_mesh_s {
	// 	   float* points;           // (X Y Z X Y Z...)
	// 	   int npoints;
	// 	   uint16_t* triangles;     // (I J K I J K...)
	// 	   int ntriangles;
	// 	   float* normals;          // Optional (X Y Z X Y Z...)
	// 	   float* tcoords;          // Optional (U V U V U V...)
	// } par_shapes_mesh;
	size_t vertex_count = cylinder->ntriangles*3;
	for(size_t i = 0; i<vertex_count; ++i) {
		// push vertex position xyz into mesh builder vertex positions
		for(size_t j = 0; j<3; ++j)
			points.push_back
				(cylinder->points[cylinder->triangles[i]*3 + j]);

		// push vertex normal(?) xyz into mesh builder normals
		for(size_t j = 0; j<3; ++j)
			normals.push_back
				(cylinder->normals[cylinder->triangles[i]*3 + j]);

		// push vertex texture coordiantes into mesh builder vertex texcoords
		for(size_t j = 0; j<2; ++j)
			texcoords.push_back(0.0f);
				// (cylinder->tcoords[cylinder->triangles[i]*2 + j]);

		// finally, the vertex color
		colors.push_back(owner->current_color().r);
		colors.push_back(owner->current_color().g);
		colors.push_back(owner->current_color().b);
		colors.push_back(owner->current_color().a);
	}
	par_shapes_free_mesh(cylinder);
}

void Turtle::MeshBuilder::add_polygon(std::vector<Vector3> poly) {
	// siano A, B, e C i primi 3 punti di points
	// se i punti sono dati in senso (TODO: controllare) orario
	// e AB e AC non sono paralleli (TODO: vedere cosa fare se lo sono)
	// allora possiamo trovare la normale del poligono come il prodotto
	// vettoriale tra AB e BC
	// https://en.wikipedia.org/wiki/Cross_product
	// (magari poi anche normalizzato)
	const Vector3 A = poly[0];
	const Vector3 B = poly[2];
	const Vector3 C = poly[3];

	const Vector3 AB = {A.x - B.x, A.y - B.y, A.z - B.z};
	const Vector3 BC = {B.x - C.x, B.y - C.y, B.z - C.z};

	// https://en.wikipedia.org/wiki/Cross_product#Computing
	// https://wikimedia.org/api/rest_v1/media/math/render/svg/a87d2b3b74e9790c36dab906d420b402a0c82230
	const Vector3 cross_prod = {
		AB.y * BC.z - AB.z - BC.y,
		AB.z * BC.x - AB.x * BC.z,
		AB.x * BC.y - AB.y * BC.x
	};


	const float cross_prod_norm = sqrt(cross_prod.x * cross_prod.x +
									   cross_prod.y * cross_prod.y +
									   cross_prod.z * cross_prod.z);
	const Vector3 norm = {
		cross_prod.x / cross_prod_norm,
		cross_prod.y / cross_prod_norm,
		cross_prod.z / cross_prod_norm,
	};

	// il poligono viene aggiunto come triangle fan
	// per farlo si aggiugono alla mesh i triangloli fatti dai vertici
	// 0 1 2
	// 0 2 3
	// 0 3 4
	// ...
	// 0 n-1 n
	// visto che le foglie sono 3d vogliamo che siano visibili da entrambi
	// i lato, disegnamo i triangoli da entrambe le direzioni
	// (poi si vede dallo shader come fare che la direzione su sia più
	//  chiara e quella giù più scura)

	for(size_t i = 1; i<poly.size()-1; i++) {
		// pusha triangolo
		// 0, i, i+1
		// e, dato il 3d
		// 0, i+1, i
		for(const auto &v : std::vector<Vector3>{
				poly[0], poly[i], poly[i+1], // triangolo su
				poly[0], poly[i+1], poly[i]  // triangolo giu
			}) {
			points.push_back(v.x);
			points.push_back(v.y);
			points.push_back(v.z);

			normals.push_back(norm.x);
			normals.push_back(norm.y);
			normals.push_back(norm.z);

			// non abbiamo texture coordinate del poligono
			// quindi ce le inventiamo
			texcoords.push_back(0.95f);
			texcoords.push_back(0.95f);

			colors.push_back(owner->current_color().r);
			colors.push_back(owner->current_color().g);
			colors.push_back(owner->current_color().b);
			colors.push_back(owner->current_color().a);
		}
	}
}

Mesh Turtle::MeshBuilder::get() {
	// TODO: no error checking lol
	Mesh mesh = {0};
	mesh.vertexCount = points.size()/3;
	mesh.triangleCount = mesh.vertexCount/3;

	mesh.vertices = points.data();
	mesh.normals = normals.data();
	mesh.texcoords = texcoords.data();
	UploadMesh(&mesh, false); // false è per dire static mesh
	return mesh;
}
