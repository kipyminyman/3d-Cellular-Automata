#define GLEW_STATIC
#include <iostream>
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <SFML/Window.hpp>
#include <SFML/System.hpp>
#include <stdlib.h>
#include <vector>
#include <set>
#include <unordered_set>

#include "utils.h"

#define FIELD_SIZE 80

// NOTE: the optimizations that make this so much better than v1 is the addition
// of face culling, obstructed occlusion, and smart ordering (based on camera)

typedef char f_t;

std::vector<GLuint> live_idx;
sf::Mutex mut;
bool did_update = false;

float x = 0;
float y = 0;
float z = 0;

int countLiveCells(const f_t (&field)[FIELD_SIZE][FIELD_SIZE][FIELD_SIZE], int x, int y, int z, int max)
{
	int ret = 0;

	if (x && y && z) { ret += field[x - 1][y - 1][z - 1] == 1; }	// UBL
	if (x && y) { ret += field[x - 1][y - 1][z] == 1; }	// BL
	if (y && z) { ret += field[x][y - 1][z - 1] == 1; }	// UB
	if (x && z) { ret += field[x - 1][y][z - 1] == 1; }	// UL
	if (x) { ret += field[x - 1][y][z] == 1; }	// L
	if (y) { ret += field[x][y - 1][z] == 1; }	// B
	if (z) { ret += field[x][y][z - 1] == 1; }	// U
	if (x && y < max - 1 && z) { ret += field[x - 1][y + 1][z - 1] == 1; }	// UFL
	if (y < max - 1 && z) { ret += field[x][y + 1][z - 1] == 1; }	// UF
	if (x && y < max - 1) { ret += field[x - 1][y + 1][z] == 1; }	// FL
	if (y < max - 1) { ret += field[x][y + 1][z] == 1; }	// F
	if (x && y && z < max - 1) { ret += field[x - 1][y - 1][z + 1] == 1; }	// DBL
	if (y && z < max - 1) { ret += field[x][y - 1][z + 1] == 1; }	// DB
	if (x && z < max - 1) { ret += field[x - 1][y][z + 1] == 1; }	// DL
	if (z < max - 1) { ret += field[x][y][z + 1] == 1; }	// D
	if (x && y < max - 1 && z < max - 1) { ret += field[x - 1][y + 1][z + 1] == 1; }	// DFL
	if (y < max - 1 && z < max - 1) { ret += field[x][y + 1][z + 1] == 1; }	// DF
	if (x < max - 1 && y && z) { ret += field[x + 1][y - 1][z - 1] == 1; }	// UBR
	if (x < max - 1 && y) { ret += field[x + 1][y - 1][z] == 1; }	// BR
	if (x < max - 1 && z) { ret += field[x + 1][y][z - 1] == 1; }	// UR
	if (x < max - 1) { ret += field[x + 1][y][z] == 1; }	// R
	if (x < max - 1 && y < max - 1 && z) { ret += field[x + 1][y + 1][z - 1] == 1; }	// UFR
	if (x < max - 1 && y < max - 1) { ret += field[x + 1][y + 1][z] == 1; }	// FR
	if (x < max - 1 && y && z < max - 1) { ret += field[x + 1][y - 1][z + 1] == 1; }	// DBR
	if (x < max - 1 && z < max - 1) { ret += field[x + 1][y][z + 1] == 1; }	// DR
	if (x < max - 1 && y < max - 1 && z < max - 1) { ret += field[x + 1][y + 1][z + 1] == 1; } // DFR

	return ret;
}

int countLiveCells_new(const f_t (&field)[FIELD_SIZE][FIELD_SIZE][FIELD_SIZE], int x, int y, int z, int max)
{
	int ret = 0;

	if (x && y && z) { ret += field[x - 1][y - 1][z - 1] == 1; }	// UBL
	if (x && y) { ret += field[x - 1][y - 1][z] == 1; }	// BL
	if (y && z) { ret += field[x][y - 1][z - 1] == 1; }	// UB
	if (x && z) { ret += field[x - 1][y][z - 1] == 1; }	// UL
	if (x) { ret += field[x - 1][y][z] == 1; }	// L
	if (y) { ret += field[x][y - 1][z] == 1; }	// B
	if (z) { ret += field[x][y][z - 1] == 1; }	// U
	if (y < max - 1) {
		ret += field[x][y + 1][z] == 1; // F
		if (x && z) { ret += field[x - 1][y + 1][z - 1] == 1; }	// UFL
		if (z) { ret += field[x][y + 1][z - 1] == 1; }	// UF
		if (x) { ret += field[x - 1][y + 1][z] == 1; }	// FL
	}
	if (z < max - 1) {
		ret += field[x][y][z + 1] == 1; // D
		if (x && y) { ret += field[x - 1][y - 1][z + 1] == 1; }	// DBL
		if (y) { ret += field[x][y - 1][z + 1] == 1; }	// DB
		if (x) { ret += field[x - 1][y][z + 1] == 1; }	// DL
		if (x && y < max - 1) { ret += field[x - 1][y + 1][z + 1] == 1; }	// DFL
		if (y < max - 1) { ret += field[x][y + 1][z + 1] == 1; }	// DF
	}
	if (x < max - 1) {
		ret += field[x + 1][y][z] == 1; // R
		if (y && z) { ret += field[x + 1][y - 1][z - 1] == 1; }	// UBR
		if (y) { ret += field[x + 1][y - 1][z] == 1; }	// BR
		if (z) { ret += field[x + 1][y][z - 1] == 1; }	// UR
		if (y < max - 1 && z) { ret += field[x + 1][y + 1][z - 1] == 1; }	// UFR
		if (y < max - 1) { ret += field[x + 1][y + 1][z] == 1; }	// FR
		if (y && z < max - 1) { ret += field[x + 1][y - 1][z + 1] == 1; }	// DBR
		if (z < max - 1) { ret += field[x + 1][y][z + 1] == 1; }	// DR
		if (y < max - 1 && z < max - 1) { ret += field[x + 1][y + 1][z + 1] == 1; } // DFR
	}

	return ret;
}

int countLiveCellsWrap(const f_t field[][FIELD_SIZE][FIELD_SIZE], int x, int y, int z, int max)
{
	int ret = 0;
	for (int i = -1; i < 2; i++) {
		for (int j = -1; j < 2; j++) {
			for (int k = -1; k < 2; k++) {
				if (i == 0 && j == 0 && k == 0) { continue; }
				ret += field[(x + i + max) % max][(y + j + max) % max][(z + k + max) % max] == 1;
			}
		}
	}
	return ret;
}

int countCloseCells(const f_t (&field)[FIELD_SIZE][FIELD_SIZE][FIELD_SIZE], int x, int y, int z, int max)
{
	int ret = 0;
	if (x) { ret += field[x - 1][y][z] != 0; }	// L
	if (y) { ret += field[x][y - 1][z] != 0; }	// B
	if (z) { ret += field[x][y][z - 1] != 0; }	// U
	if (y < max - 1) { ret += field[x][y + 1][z] != 0; }	// F
	if (z < max - 1) { ret += field[x][y][z + 1] != 0; }	// D
	if (x < max - 1) { ret += field[x + 1][y][z] != 0; }	// R

	return ret;
}

void stepAutomata_old(f_t (&field)[FIELD_SIZE][FIELD_SIZE][FIELD_SIZE])
{
	//sf::Clock clock;
	//clock.restart();

	std::vector<int> living_cells;
	std::vector<int> dying_cells;

	for (unsigned i = 0; i < FIELD_SIZE; i++) {
		for (unsigned j = 0; j < FIELD_SIZE; j++) {
			for (unsigned k = 0; k < FIELD_SIZE; k++) {
				int live_count = countLiveCells_new(field, i, j, k, FIELD_SIZE);
				if (field[i][j][k] == 1 && live_count >= 4 && live_count <= 7) { // survive
					living_cells.push_back((i << 20) + (j << 10) + k);
				} else if (field[i][j][k] == 0 && live_count >= 6 && live_count <= 8) { // born
					living_cells.push_back((i << 20) + (j << 10) + k);
				} else if (field[i][j][k] != 0) { // die
					dying_cells.push_back((i << 20) + (j << 10) + k);
				}
			}
		}
	}

	for (unsigned i = 0; i < living_cells.size(); i++) {
		unsigned id = living_cells[i];
		field[id >> 20][(id << 12) >> 22][(id << 22) >> 22] = 1;
	}

	for (unsigned i = 0; i < dying_cells.size(); i++) {
		unsigned id = dying_cells[i];
		field[id >> 20][(id << 12) >> 22][(id << 22) >> 22] = (field[id >> 20][(id << 12) >> 22][(id << 22) >> 22] + 1) % 10;
	}

	//auto tmp = clock.getElapsedTime().asSeconds();
	//std::cout << tmp << std::endl;
}

/*void stepAutomata(f_t field[][FIELD_SIZE][FIELD_SIZE])
{
	sf::Clock clock;
	clock.restart();

	std::vector<int> living_cells;
	std::vector<int> dying_cells;

	if (check_idx.size() == 0) { // itterate through all
		std::cout << "call me once" << std::endl;
		for (unsigned i = 0; i < FIELD_SIZE; i++) {
			for (unsigned j = 0; j < FIELD_SIZE; j++) {
				for (unsigned k = 0; k < FIELD_SIZE; k++) {
					int live_count = countLiveCells(field, i, j, k, FIELD_SIZE);
					if (field[i][j][k] == 1 && live_count >= 4 && live_count <= 7) { // survive
						living_cells.push_back((i << 20) + (j << 10) + k);
					} else if (field[i][j][k] == 0 && live_count >= 6 && live_count <= 8) { // born
						living_cells.push_back((i << 20) + (j << 10) + k);
					} else if (field[i][j][k] != 0) { // die
						dying_cells.push_back((i << 20) + (j << 10) + k);
					}
				}
			}
		}
	} else {
		for (auto it = check_idx.begin(); it != check_idx.end(); ++it) {
			GLuint id = *it;
			int i = id >> 20;
			int j = (id << 12) >> 22;
			int k = (id << 22) >> 22;

			int live_count = countLiveCells(field, i, j, k, FIELD_SIZE);
			if (field[i][j][k] == 1 && live_count >= 4 && live_count <= 7) { // survive
				living_cells.push_back((i << 20) + (j << 10) + k);
			} else if (field[i][j][k] == 0 && live_count >= 6 && live_count <= 8) { // born
				living_cells.push_back((i << 20) + (j << 10) + k);
			} else if (field[i][j][k] != 0) { // die
				dying_cells.push_back((i << 20) + (j << 10) + k);
			}
		}

		check_idx = {};
	}

	auto tmp = clock.getElapsedTime().asSeconds();

	bool field_check[FIELD_SIZE][FIELD_SIZE][FIELD_SIZE];
	memset(field_check, 0, sizeof(field_check));

	for (unsigned i = 0; i < living_cells.size(); i++) {
		unsigned id = living_cells[i];
		int cell_x = id >> 20;
		int cell_y = (id << 12) >> 22;
		int cell_z = (id << 22) >> 22;

		field[cell_x][cell_y][cell_z] = 1;

		for (int j = -1; j <= 1; j++) {
			for (int k = -1; k <= 1; k++) {
				for (int m = -1; m <= 1; m++) {
					if (cell_x + j < 0 || cell_x + j >= FIELD_SIZE ||
						cell_y + k < 0 || cell_y + k >= FIELD_SIZE ||
						cell_z + m < 0 || cell_z + m >= FIELD_SIZE) { continue; }

					if (!field_check[cell_x + j][cell_y + k][cell_z + m]) {
						check_idx.push_back(((cell_x + j) << 20) + ((cell_y + k) << 10) + cell_z + m);
						field_check[cell_x + j][cell_y + k][cell_z + m] = true;
					}
				}
			}
		}
	}

	for (unsigned i = 0; i < dying_cells.size(); i++) {
		unsigned id = dying_cells[i];
		int cell_x = id >> 20;
		int cell_y = (id << 12) >> 22;
		int cell_z = (id << 22) >> 22;

		field[cell_x][cell_y][cell_z] = (field[cell_x][cell_y][cell_z] + 1) % 10;

		for (int j = -1; j <= 1; j++) {
			for (int k = -1; k <= 1; k++) {
				for (int m = -1; m <= 1; m++) {
					if (cell_x + j < 0 || cell_x + j >= FIELD_SIZE ||
						cell_y + k < 0 || cell_y + k >= FIELD_SIZE ||
						cell_z + m < 0 || cell_z + m >= FIELD_SIZE) { continue; }

					if (!field_check[cell_x + j][cell_y + k][cell_z + m]) {
						check_idx.push_back(((cell_x + j) << 20) + ((cell_y + k) << 10) + cell_z + m);
						field_check[cell_x + j][cell_y + k][cell_z + m] = true;
					}
				}
			}
		}
	}

	//auto tmp = clock.getElapsedTime().asSeconds();
	std::cout << tmp << std::endl;
}
*/

std::vector<unsigned> getDrawableCells(const f_t (&field)[FIELD_SIZE][FIELD_SIZE][FIELD_SIZE])
{
	std::vector<unsigned> ret;

	int i_start = 0;
	int k_start = 0;
	int i_inc = 1;
	int k_inc = 1;
	int i_goal = FIELD_SIZE;
	int k_goal = FIELD_SIZE;

	if (z > 0) {//sf::Keyboard::isKeyPressed(sf::Keyboard::RShift)) {//z > 0) {
		k_start = FIELD_SIZE - 1;
		k_inc = -1;
		k_goal = -1;
	}
	if (x > 0) {//sf::Keyboard::isKeyPressed(sf::Keyboard::LShift)) {//x > 0) {
		i_start = FIELD_SIZE - 1;
		i_inc = -1;
		i_goal = -1;
	}

	for (int i = i_start; i != i_goal; i += i_inc) {//for (int i = FIELD_SIZE - 1; i >= 0; i--) {
		for (int j = FIELD_SIZE - 1; j >= 0; j--) { // better
			for (int k = k_start; k != k_goal; k += k_inc) {//for (int k = FIELD_SIZE - 1; k >= 0; k--) {
				if (field[i][j][k] && countCloseCells(field, i, j, k, FIELD_SIZE) != 6) { ret.push_back((i << 20) + (j << 10) + k); }
			}
		}
	}

	return ret;
}


void threadHandler()
{
	f_t field[FIELD_SIZE][FIELD_SIZE][FIELD_SIZE];
	memset(field, 0, sizeof(field));

	//srand(time(NULL));

	for (int i = 0; i < 50; i++) {
		for (int j = 0; j < 50; j++) {
			for (int k = 0; k < 50; k++) {
				field[i][j][k] = rand()%2;
			}
		}
	}

	while (!sf::Keyboard::isKeyPressed(sf::Keyboard::Q)) {
		stepAutomata_old(field);
		std::vector<GLuint> live_idxx = getDrawableCells(field);

		mut.lock();
		live_idx = live_idxx;
		did_update = true;
		mut.unlock();

		//break;
	}
}


int main() {
	float width = 2000;
	float height = 1600;

	sf::ContextSettings cs(24, 8);
	sf::Window window(sf::VideoMode(width, height), "tak3over", sf::Style::Default, cs);
	//window.setFramerateLimit(60);

	const GLubyte* graph_card = glGetString(GL_VENDOR);
    std::cout << "Current graphics card: " << graph_card << std::endl;
    const GLubyte* ver = glGetString(GL_VERSION);
    std::cout << "OpenGL version: " << ver << std::endl;


	// glew init stuff //
	glewExperimental = GL_TRUE;
	glewInit();
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);


	// vbo stuff //
	float cube[] = {
		// Down
		0.0f, 0.0f, 0.0f,
		1.0f, 0.0f, 0.0f,
		1.0f, 0.0f, 1.0f,
		0.0f, 0.0f, 0.0f,
		1.0f, 0.0f, 1.0f,
		0.0f, 0.0f, 1.0f,

		// Up
		0.0f, 1.0f, 0.0f,
		1.0f, 1.0f, 1.0f,//
		1.0f, 1.0f, 0.0f,//
		0.0f, 1.0f, 0.0f,
		0.0f, 1.0f, 1.0f,//
		1.0f, 1.0f, 1.0f,//

		// Back
		0.0f, 0.0f, 0.0f,
		1.0f, 1.0f, 0.0f,//
		1.0f, 0.0f, 0.0f,//
		0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f,//
		1.0f, 1.0f, 0.0f,//

		// Front
		0.0f, 0.0f, 1.0f,
		1.0f, 0.0f, 1.0f,
		1.0f, 1.0f, 1.0f,
		0.0f, 0.0f, 1.0f,
		1.0f, 1.0f, 1.0f,
		0.0f, 1.0f, 1.0f,

		// Left
		0.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f,
		0.0f, 1.0f, 1.0f,
		0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 1.0f,
		0.0f, 1.0f, 0.0f,

		// Right
		1.0f, 0.0f, 0.0f,
		1.0f, 1.0f, 1.0f,//
		1.0f, 0.0f, 1.0f,//
		1.0f, 0.0f, 0.0f,
		1.0f, 1.0f, 0.0f,//
		1.0f, 1.0f, 1.0f,//
	};
	float normal[] = {
		0.0f, -1.0f, 0.0f,
		0.0f, -1.0f, 0.0f,
		0.0f, -1.0f, 0.0f,
		0.0f, -1.0f, 0.0f,
		0.0f, -1.0f, 0.0f,
		0.0f, -1.0f, 0.0f,

		0.0f, 1.0f, 0.0f,
		0.0f, 1.0f, 0.0f,
		0.0f, 1.0f, 0.0f,
		0.0f, 1.0f, 0.0f,
		0.0f, 1.0f, 0.0f,
		0.0f, 1.0f, 0.0f,

		0.0f, 0.0f, -1.0f,
		0.0f, 0.0f, -1.0f,
		0.0f, 0.0f, -1.0f,
		0.0f, 0.0f, -1.0f,
		0.0f, 0.0f, -1.0f,
		0.0f, 0.0f, -1.0f,

		0.0f, 0.0f, 1.0f,
		0.0f, 0.0f, 1.0f,
		0.0f, 0.0f, 1.0f,
		0.0f, 0.0f, 1.0f,
		0.0f, 0.0f, 1.0f,
		0.0f, 0.0f, 1.0f,

		-1.0f, 0.0f, 0.0f,
		-1.0f, 0.0f, 0.0f,
		-1.0f, 0.0f, 0.0f,
		-1.0f, 0.0f, 0.0f,
		-1.0f, 0.0f, 0.0f,
		-1.0f, 0.0f, 0.0f,

		1.0f, 0.0f, 0.0f,
		1.0f, 0.0f, 0.0f,
		1.0f, 0.0f, 0.0f,
		1.0f, 0.0f, 0.0f,
		1.0f, 0.0f, 0.0f,
		1.0f, 0.0f, 0.0f
	};

	GLuint vbo_vert = allocateBuffer(GL_ARRAY_BUFFER, sizeof(cube), cube, GL_STATIC_DRAW);
	GLuint vbo_norm = allocateBuffer(GL_ARRAY_BUFFER, sizeof(normal), normal, GL_STATIC_DRAW);
	GLuint vbo_instance_trans_id = allocateBuffer(GL_ARRAY_BUFFER, sizeof(GLuint) * FIELD_SIZE * FIELD_SIZE * FIELD_SIZE, NULL, GL_STREAM_DRAW);

	// Shader stuff //
	GLuint vertex_shader = createShader(GL_VERTEX_SHADER, "vertex_shader.vert");
	checkShaderStatus(vertex_shader);
	GLuint fragment_shader = createShader(GL_FRAGMENT_SHADER, "fragment_shader.frag");
	checkShaderStatus(fragment_shader);

	GLuint shaders[] = { vertex_shader, fragment_shader };
	GLuint shader_program = createShaderProgram(shaders, 2);


	// setup attributes //
	GLint pos_attr = setupVertexAttribute(vbo_vert, shader_program, "pos", 3, GL_FLOAT, 0, 0);
	GLint norm_attr = setupVertexAttribute(vbo_norm, shader_program, "norm", 3, GL_FLOAT, 0, 0);
	GLint inst_attr_trans_id = setupVertexAttribute(vbo_instance_trans_id, shader_program, "trans_id", 1, GL_UNSIGNED_INT, 0, 0);
	glVertexAttribDivisor(inst_attr_trans_id, 1);

	// setup perspective uniform //
	GLint uniProj = glGetUniformLocation(shader_program, "proj");
	setPerspective(uniProj, glm::radians(45.0f), width / height, .1f, 500.0f);

	// setup view (where look) uniform //
	GLint uniView = glGetUniformLocation(shader_program, "view");
	setView(uniView, 0, 0, 0, /**/ 0, 0);

	float move_speed = .05;
	float dir = 0;
	float rad = 2 * FIELD_SIZE;

	sf::Clock clock;
    float total_time = 0;
	unsigned total_updates = 0;
    unsigned count = 0;

	sf::Thread thread(&threadHandler);
	thread.launch();

	while (window.isOpen()) {
		clock.restart();

		int s;
		if ((s = glGetError())) { std::cerr << "uhoh: " << s << std::endl; }

		x = rad * cos(dir);
		y = rad / 2.0;
		z = rad * sin(dir);
		dir += move_speed;

		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down)) { rad += 1; }
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up)) { rad -= 1; }
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left)) { move_speed += .005; }
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right)) { move_speed -= .005; }
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Space)) { move_speed = 0; }

		/*if (sf::Keyboard::isKeyPressed(sf::Keyboard::W)) {
			z -= move_speed * cos(glm::radians(dir));
			x += move_speed * sin(glm::radians(dir));
		}
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::S)) {
			z += move_speed * cos(glm::radians(dir));
			x -= move_speed * sin(glm::radians(dir));
		}
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::A)) {
			z -= move_speed * sin(glm::radians(dir));
			x -= move_speed * cos(glm::radians(dir));
		}
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::D)) {
			z += move_speed * sin(glm::radians(dir));
			x += move_speed * cos(glm::radians(dir));
		}
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Space)) { y += .1; }
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::LShift)) { y -= .1; }
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up)) { pitch -= 5; }
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down)) { pitch += 5; }
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left)) { dir -= 5; }
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right)) { dir += 5; }*/

		setView(uniView, x, y, z, 0, 0, 0);

        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) { window.close(); }
        }

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		mut.lock();
		size_t tmp_size = live_idx.size();
		if (did_update) {
			glNamedBufferSubData(vbo_instance_trans_id, 0, sizeof(GLuint) * live_idx.size(), live_idx.data());
			total_updates++;
			did_update = false;
		}
		mut.unlock();

		glDrawArraysInstanced(GL_TRIANGLES, 0, sizeof(cube) / (3 * sizeof(float)), tmp_size);
		window.display();

		// FPS stuff //
		total_time += clock.getElapsedTime().asSeconds();
        count++;
        if (total_time >= 1) {
            float fps = float(count) / total_time;
            std::cout << "fps: " << fps << " (" << count << " / " << total_time << ")" << std::endl;
			std::cout << "Coords: " << x << ", " << y << ", " << z << std::endl;
			std::cout << "ticks/s: " << float(total_updates) / total_time << std::endl;

            total_time = 0;
			total_updates = 0;
            count = 0;
        }
	}

	std::cout << "Hello, world!" << std::endl;
	return 0;
}
