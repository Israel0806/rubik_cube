#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "shader.h"
#include "camera.h"

#include "Rubik.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <iostream>
#include <time.h>
#include <thread>
using namespace std;
using namespace glm;

//string path = "/home/sebas/Documents/7 SEMESTER/COMPUTER GRAPHICS/SEMESTRE_PART2/GLFW_GLAD_GLUT_GLEW_cmake_project_Windows_Linux/GLFW_GLAD_GLUT_GLEW_cmake_project/src/rubik_cube/";
string path = "D:/UCSP/Semestres/Semestre_VII/Computacion_Grafica/Proyecto_final/src/rubik_cube/";
int screenWidth = 800, screenHeight = 600;
unsigned int VBO[729], VAO[729], lightVBO, lightVAO;
Shader *shader, *lightShader;
mat4 model, view, projection;

// mouse
bool rightClick = false;

// timing
float deltaTime = 0.0f;	// Time between current frame and last frame
float lastFrame = 0.0f; // Time of last frame

float lastX = screenWidth / 2, lastY = screenHeight / 2;
bool firstMouse = true;

Camera camera (vec3 (0.0f, 2.0f, 15.0f));

// textures
// S = 0, U = 1, P = 2, C = 3, G = 4
unsigned int textures[5];

// lighting
vec3 lightPos (10.0f, 0.0f, 5.0f);

//clase cubo 
class cubo {
public:
	int size = 396;
	mat4 model1 = mat4 (1.0f);
	int id;
	int numero_pintados = 0;
	int carax = 2, caray = 2, caraz = 2;
	int caratx = -1, caraty = -1, caratz = -1;
	vec3 direccion;
	vec3 offset;
	float vertices[396];
	cubo () {}
	cubo (int id_) {
		id = id_;
	}
	void set_id (int id_) {
		id = id_;
	}

	void actualizar_giros () {
		//cout << "YYYYYxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n";
		//cout << "id_b:: " << id << " x: " << carax << " y: " << caray << " z: " << caraz << "\n";
		carax = (int)direccion.x;
		caray = (int)direccion.y;
		caraz = (int)direccion.z;
		//cout << "id_a:: " << id << " x: " << carax << " y: " << caray << " z: " << caraz << "\n";
	}
	void bufferConfigCube () {
		glBindBuffer (GL_ARRAY_BUFFER, VBO[id]);
		glBufferData (GL_ARRAY_BUFFER, sizeof (vertices), vertices, GL_DYNAMIC_DRAW);
		glBindVertexArray (VAO[id]);

		// coordinates
		glVertexAttribPointer (0, 3, GL_FLOAT, GL_FALSE, 11 * sizeof (float), (void *)0);
		glEnableVertexAttribArray (0);

		// normal vector
		glVertexAttribPointer (1, 3, GL_FLOAT, GL_FALSE, 11 * sizeof (float), (void *)(3 * sizeof (float)));
		glEnableVertexAttribArray (1);

		// color 
		glVertexAttribPointer (2, 3, GL_FLOAT, GL_FALSE, 11 * sizeof (float), (void *)(6 * sizeof (float)));
		glEnableVertexAttribArray (2);

		//texture
		glVertexAttribPointer (3, 2, GL_FLOAT, GL_FALSE, 11 * sizeof (float), (void *)(9 * sizeof (float)));
		glEnableVertexAttribArray (3);
	}

	void justRotate (vec3 dir) {
		mat4 matrix;
		matrix = mat4 (1.0f);
		matrix = translate (matrix, dir);
		vec3 transformed;
		for ( int j = 0; j < size; j += 11 ) {
			transformed = vec3 (matrix * vec4 (vec3 (vertices[j], vertices[j + 1], vertices[j + 2]), 1.0f));
			vertices[j] = transformed.x;
			vertices[j + 1] = transformed.y;
			vertices[j + 2] = transformed.z;
		}
		//for ( int j = 0; j < size; j += 11 ) {
		//	vec3 transformed = vec3 (matrix * vec4 (vertices[j], vertices[j + 1], vertices[j + 2], 1.0));
		//	vertices[j] = transformed.x;
		//	vertices[j + 1] = transformed.y;
		//	vertices[j + 2] = transformed.z;
		//}
		glBindBuffer (GL_ARRAY_BUFFER, VBO[id]);
		// get pointer
		void *ptr = glMapBuffer (GL_ARRAY_BUFFER, GL_WRITE_ONLY);
		// now copy data into memory
		memcpy (ptr, vertices, sizeof (vertices));
		// make sure to tell OpenGL we're done with the pointer
		glUnmapBuffer (GL_ARRAY_BUFFER);
	}
	void translateExploit (vec3 to) {
		mat4 matrix = mat4 (1.0f);
		mat4 matrixTo = translate (matrix, to);
		vec3 transformed;
		offset = offset + to;

		for ( int j = 0; j < size; j += 11 ) {
			transformed = vec3 (matrixTo * vec4 (vertices[j], vertices[j + 1], vertices[j + 2], 1.0f));

			vertices[j] = transformed.x;
			vertices[j + 1] = transformed.y;
			vertices[j + 2] = transformed.z;
		}

		glBindBuffer (GL_ARRAY_BUFFER, VBO[id]);
		// get pointer
		void *ptr = glMapBuffer (GL_ARRAY_BUFFER, GL_WRITE_ONLY);
		// now copy data into memory
		memcpy (ptr, vertices, sizeof (vertices));
		// make sure to tell OpenGL we're done with the pointer
		glUnmapBuffer (GL_ARRAY_BUFFER);
	}

	void move (vec3 giro, int angleMovement, bool all) {
		mat4 matrix = mat4 (1.0f);
		mat4 matrixToOrigin = translate (matrix, -offset);
		mat4 matrixBack = translate (matrix, offset);
		matrix = rotate (matrix, radians (5.0f), giro);
		vec3 transformed;
		if ( all ) {
			for ( int j = 0; j < size; j += 11 ) {
				transformed = vec3 (matrix * vec4 (vertices[j], vertices[j + 1], vertices[j + 2], 1.0f));

				vertices[j] = transformed.x;
				vertices[j + 1] = transformed.y;
				vertices[j + 2] = transformed.z;
			}
		} else {
			//cout << "Offset: " << offset.x<<","<<offset.y<<","<<offset.z << endl;
			for ( int j = 0; j < size; j += 11 ) {
				transformed = vec3 (vertices[j], vertices[j + 1], vertices[j + 2]);

				transformed = vec3 (matrixToOrigin * vec4 (transformed, 1.0f));
				transformed = vec3 (matrix * vec4 (transformed, 1.0f));
				transformed = vec3 (matrixBack * vec4 (transformed, 1.0f));

				vertices[j] = transformed.x;
				vertices[j + 1] = transformed.y;
				vertices[j + 2] = transformed.z;
			}

		}
		glBindBuffer (GL_ARRAY_BUFFER, VBO[id]);
		// get pointer
		void *ptr = glMapBuffer (GL_ARRAY_BUFFER, GL_WRITE_ONLY);
		// now copy data into memory
		memcpy (ptr, vertices, sizeof (vertices));
		// make sure to tell OpenGL we're done with the pointer
		glUnmapBuffer (GL_ARRAY_BUFFER);
		if ( angleMovement == 90 ) {
			matrix = mat4 (1.0f);
			matrix = rotate (matrix, radians (90.0f), giro);
			direccion = vec3 (matrix * vec4 (direccion, 1.0));
			if ( all ) {
				offset = vec3 (matrix * vec4 (offset, 1.0));
			}
			actualizar_giros ();
		}
	}

	void move_x_positive (int angleMovement, bool all = false) {
		move (vec3 (1.0f, 0.0f, 0.0f), angleMovement, all);
	}

	void move_x_negative (int angleMovement, bool all = false) {
		move (vec3 (-1.0f, 0.0f, 0.0f), angleMovement, all);
	}

	void move_y_positive (int angleMovement, bool all = false) {
		move (vec3 (0.0f, 1.0f, 0.0f), angleMovement, all);
	}

	void move_y_negative (int angleMovement, bool all = false) {
		move (vec3 (0.0f, -1.0f, 0.0f), angleMovement, all);
	}

	void move_z_positive (int angleMovement, bool all = false) {
		move (vec3 (0.0f, 0.0f, 1.0f), angleMovement, all);
	}

	void move_z_negative (int angleMovement, bool all = false) {
		move (vec3 (0.0f, 0.0f, -1.0f), angleMovement, all);
	}

	void explotar (vec3 dir) {
		//model1 = translate (model1, vec3 (dir.x * 3, dir.y * 3, dir.z * 3));
		translateExploit (vec3 (dir.x * 3, dir.y * 3, dir.z * 3));
	}
	void desexplotar (vec3 dir) {
		translateExploit (vec3 (dir.x * -3, dir.y * -3, dir.z * -3));
	}

	void draw () {
		glBindVertexArray (VAO[id]);
		shader->setMat4 ("model", model1);

		glBindTexture (GL_TEXTURE_2D, caratz);
		glDrawArrays (GL_TRIANGLES, 0, 12);

		glBindTexture (GL_TEXTURE_2D, caratx);
		glDrawArrays (GL_TRIANGLES, 12, 12);

		glBindTexture (GL_TEXTURE_2D, caraty);
		glDrawArrays (GL_TRIANGLES, 24, 12);
	}
};

//clase del cubo rubik
class cubeRubik {
public:
	string moves, movesSolver;
	int angleMovement;
	vec3 offset;
	const int size = 396;
	cubo cubesAround[27];
	//cubo *cubesAround ;
	int carax = 2, caray = 2, caraz = 2;
	vec3 direccion;

	bool all, allSolve;
	bool teclaMov[12];
	bool solveRubik, shuffle, shuffle2, moving;

	bool explode, deExplode;

	float vertices[396] = {
		/// Orden: Back, Front, Left,  Right, Down, Up
		/// Coordenadas       // Normales	       // Color            //texture
		/// Back: Amarillo-0 -z
		 0.5f,  0.5f, -0.5f,   0.0f,  0.0f, -1.0f,   1.0f, 1.0f, 0.0f,   0.666f, 0.666f,
		 0.5f, -0.5f, -0.5f,   0.0f,  0.0f, -1.0f,   1.0f, 1.0f, 0.0f,   0.666f, 0.333f,
		-0.5f, -0.5f, -0.5f,   0.0f,  0.0f, -1.0f,   1.0f, 1.0f, 0.0f,   0.333f, 0.333f,
		-0.5f, -0.5f, -0.5f,   0.0f,  0.0f, -1.0f,   1.0f, 1.0f, 0.0f,   0.333f, 0.333f,
		-0.5f,  0.5f, -0.5f,   0.0f,  0.0f, -1.0f,   1.0f, 1.0f, 0.0f,   0.333f, 0.666f,
		 0.5f,  0.5f, -0.5f,   0.0f,  0.0f, -1.0f,   1.0f, 1.0f, 0.0f,   0.666f, 0.666f,

		 /// Front: Blanco-1  +z
		 -0.5f, -0.5f,  0.5f,   0.0f,  0.0f, 1.0f,    1.0f, 1.0f, 1.0f,   0.333f, 0.333f,
		  0.5f, -0.5f,  0.5f,   0.0f,  0.0f, 1.0f,    1.0f, 1.0f, 1.0f,   0.666f, 0.333f,
		  0.5f,  0.5f,  0.5f,   0.0f,  0.0f, 1.0f,    1.0f, 1.0f, 1.0f,   0.666f, 0.666f,
		  0.5f,  0.5f,  0.5f,   0.0f,  0.0f, 1.0f,    1.0f, 1.0f, 1.0f,   0.666f, 0.666f,
		 -0.5f,  0.5f,  0.5f,   0.0f,  0.0f, 1.0f,    1.0f, 1.0f, 1.0f,   0.333f, 0.666f,
		 -0.5f, -0.5f,  0.5f,   0.0f,  0.0f, 1.0f,    1.0f, 1.0f, 1.0f,   0.333f, 0.333f,

		 /// Left: Naranja-2  -x
		 -0.5f,  0.5f,  0.5f,  -1.0f,  0.0f,  0.0f,   1.0f, 0.5f, 0.0f,   0.333f, 0.333f,
		 -0.5f,  0.5f, -0.5f,  -1.0f,  0.0f,  0.0f,   1.0f, 0.5f, 0.0f,   0.666f, 0.333f,
		 -0.5f, -0.5f, -0.5f,  -1.0f,  0.0f,  0.0f,   1.0f, 0.5f, 0.0f,   0.666f, 0.666f,
		 -0.5f, -0.5f, -0.5f,  -1.0f,  0.0f,  0.0f,   1.0f, 0.5f, 0.0f,   0.666f, 0.666f,
		 -0.5f, -0.5f,  0.5f,  -1.0f,  0.0f,  0.0f,   1.0f, 0.5f, 0.0f,   0.333f, 0.666f,
		 -0.5f,  0.5f,  0.5f,  -1.0f,  0.0f,  0.0f,   1.0f, 0.5f, 0.0f,   0.333f, 0.333f,

		 /// Right: Red-3     +x
		  0.5f, -0.5f, -0.5f,   1.0f,  0.0f,  0.0f,   1.0f, 0.0f, 0.0f,   0.666f, 0.666f,
		  0.5f,  0.5f, -0.5f,   1.0f,  0.0f,  0.0f,   1.0f, 0.0f, 0.0f,   0.666f, 0.333f,
		  0.5f,  0.5f,  0.5f,   1.0f,  0.0f,  0.0f,   1.0f, 0.0f, 0.0f,   0.333f, 0.333f,
		  0.5f,  0.5f,  0.5f,   1.0f,  0.0f,  0.0f,   1.0f, 0.0f, 0.0f,   0.333f, 0.333f,
		  0.5f, -0.5f,  0.5f,   1.0f,  0.0f,  0.0f,   1.0f, 0.0f, 0.0f,   0.333f, 0.666f,
		  0.5f, -0.5f, -0.5f,   1.0f,  0.0f,  0.0f,   1.0f, 0.0f, 0.0f,   0.666f, 0.666f,

		  /// Down: Blue-4   -y
		 -0.5f, -0.5f, -0.5f,   0.0f, -1.0f,  0.0f,   0.0f, 0.0f, 1.0f,   0.333f, 0.333f,
		  0.5f, -0.5f, -0.5f,   0.0f, -1.0f,  0.0f,   0.0f, 0.0f, 1.0f,   0.666f, 0.333f,
		  0.5f, -0.5f,  0.5f,   0.0f, -1.0f,  0.0f,   0.0f, 0.0f, 1.0f,   0.666f, 0.666f,
		  0.5f, -0.5f,  0.5f,   0.0f, -1.0f,  0.0f,   0.0f, 0.0f, 1.0f,   0.666f, 0.666f,
		 -0.5f, -0.5f,  0.5f,   0.0f, -1.0f,  0.0f,   0.0f, 0.0f, 1.0f,   0.333f, 0.666f,
		 -0.5f, -0.5f, -0.5f,   0.0f, -1.0f,  0.0f,   0.0f, 0.0f, 1.0f,   0.333f, 0.333f,

		 /// Up: Verde-5     +y
		  0.5f,  0.5f,  0.5f,   0.0f,  1.0f,  0.0f,   0.0f, 1.0f, 0.0f,   0.666f, 0.666f,
		  0.5f,  0.5f, -0.5f,   0.0f,  1.0f,  0.0f,   0.0f, 1.0f, 0.0f,   0.666f, 0.333f,
		 -0.5f,  0.5f, -0.5f,   0.0f,  1.0f,  0.0f,   0.0f, 1.0f, 0.0f,   0.333f, 0.333f,
		 -0.5f,  0.5f, -0.5f,   0.0f,  1.0f,  0.0f,   0.0f, 1.0f, 0.0f,   0.333f, 0.333f,
		 -0.5f,  0.5f,  0.5f,   0.0f,  1.0f,  0.0f,   0.0f, 1.0f, 0.0f,   0.333f, 0.666f,
		  0.5f,  0.5f,  0.5f,   0.0f,  1.0f,  0.0f,   0.0f, 1.0f, 0.0f,   0.666f, 0.666f,
	};
	float verticesAux[396];

	vec3 cubePositions[27] = {
		/// de espacio = 0.1
		// CENTER  -0.5 a 0.5
		/// UP
		vec3 (0.0f,  1.1f,  1.1f), // center up
		vec3 (0.f,   1.1f,  0.0f), // center
		vec3 (0.0f,  1.1f, -1.1f), // center down
		vec3 (1.1f,  1.1f,  0.0f), // right mid
		vec3 (1.1f,  1.1f,  1.1f), // right up
		vec3 (1.1f,  1.1f, -1.1f), // right down
		vec3 (-1.1f, 1.1f,  0.0f), // left mid
		vec3 (-1.1f, 1.1f,  1.1f), // left up
		vec3 (-1.1f, 1.1f, -1.1f), // left down

		// MIDDLE
		vec3 (0.0f,  0.0f,  1.1f), // center up
		vec3 (0.0f,  0.0f,  0.0f), //center 
		vec3 (0.0f,  0.0f, -1.1f), // center down
		vec3 (1.1f,  0.0f,  0.0f), // right up
		vec3 (1.1f,  0.0f,  1.1f), // right mid
		vec3 (1.1f,  0.0f, -1.1f), // right down
		vec3 (-1.1f, 0.0f,  0.0f), // left up
		vec3 (-1.1f, 0.0f,  1.1f), // left mid
		vec3 (-1.1f, 0.0f, -1.1f), // left down

		// BOTTOM
		vec3 (1.1f,-1.1f,0.f), // center up
		vec3 (0.0f,-1.1f,0.f), // center mid
		vec3 (-1.1f,-1.1f,0.f), // center left
		vec3 (1.1f,-1.1f,-1.1f), // left up  
		vec3 (0.0f,-1.1f,-1.1f), // left mid  
		vec3 (-1.1f,-1.1f,-1.1f), // left down
		vec3 (1.1f,-1.1f,1.1f), // right up  
		vec3 (0.0f,-1.1f,1.1f), // right mid  
		vec3 (-1.1f,-1.1f,1.1f) // right left
	};

	void create (vec3 offset, int id) {
		//cubesAround = new cubo[27];
		for ( int i = 0; i < 12; ++i )
			teclaMov[i] = false;
		moves = "";
		this->offset = offset;
		solveRubik = shuffle = shuffle2 = moving = all = allSolve = explode = deExplode = false;

		carax = caray = caraz = 0;
		carax = (int)offset.x > 0 ? 1 : (int)offset.x < 0 ? -1 : 0;
		caray = (int)offset.y > 0 ? 1 : (int)offset.y < 0 ? -1 : 0;
		caraz = (int)offset.z > 0 ? 1 : (int)offset.z < 0 ? -1 : 0;

		this->direccion.x = (int)offset.x > 0 ? 1.1 : (int)offset.x < 0 ? -1.1 : 0;
		this->direccion.y = (int)offset.y > 0 ? 1.1 : (int)offset.y < 0 ? -1.1 : 0;
		this->direccion.z = (int)offset.z > 0 ? 1.1 : (int)offset.z < 0 ? -1.1 : 0;

		angleMovement = 0;
		createCubes (id);
	}

	void actualizar_giros () {
		carax = (int)(this->direccion.x);
		caray = (int)(this->direccion.y);
		caraz = (int)(this->direccion.z);
	}

	void actualizar_cubos_dir () {
		for ( int i = 0; i < 27; ++i )
			cubesAround[i].actualizar_giros ();
	}

	void moveFace (string &moves) {
		if ( moves[0] ) {
			if ( moves.size () > 1 && moves[1] == '2' )
				moves[1] = moves[0];
			switch ( moves[0] ) {
			case 'R':
				if ( moves[1] == '\'' )
					teclaMov[1] = true;
				else
					teclaMov[0] = true;
				break;
			case 'L':
				if ( moves[1] == '\'' )
					teclaMov[3] = true;
				else
					teclaMov[2] = true;
				break;
			case 'B':
				if ( moves[1] == '\'' )
					teclaMov[5] = true;
				else
					teclaMov[4] = true;
				break;
			case 'F':
				if ( moves[1] == '\'' )
					teclaMov[7] = true;
				else
					teclaMov[6] = true;
				break;
			case 'U':
				if ( moves[1] == '\'' )
					teclaMov[9] = true;
				else
					teclaMov[8] = true;
				break;
			case 'D':
				if ( moves[1] == '\'' )
					teclaMov[11] = true;
				else
					teclaMov[10] = true;
				break;
			default:
				break;
			}
			moves = moves.substr (1);
		} else {
			if ( !shuffle2 ) {
				this->moves = "";
				if ( !allSolve )
					deExplode = true;
			} else
				shuffle2 = false;
			solveRubik = allSolve = false;
		}
	}

	void createCubes (int id) {
		//cubesAround = new cubo[27];
		//-1,0,1
		bool x[3];
		bool y[3];
		bool z[3];
		for ( int i = 0; i < 27; ++i ) {
			int caratx = -1, caraty = -1, caratz = -1;
			//// copy array
			for ( int k = 0; k < size; ++k )
				verticesAux[k] = vertices[k];

			for ( int k = 0; k < 3; k++ ) {
				x[k] = false;
				y[k] = false;
				z[k] = false;
			}

			//cout << cubePositions[i].x << " / " << cubePositions[i].y << " / " << cubePositions[i].z << "\n";

			/// direcciones
			/// 0 = negative axis
			/// 1 = black face
			/// 2 = positive axis
			if ( ((int)cubePositions[i].x) == -1 )
				x[0] = true;
			else if ( ((int)cubePositions[i].x) == 1 )
				x[2] = true;

			if ( ((int)cubePositions[i].y) == -1 )
				y[0] = true;
			else if ( (int)cubePositions[i].y == 1 )
				y[2] = true;

			if ( ((int)cubePositions[i].z) == -1 )
				z[0] = true;
			else if ( ((int)cubePositions[i].z) == 1 )
				z[2] = true;


			/// n is for color
			/// n2 is for texture coordinate?
			int n = 6, n2 = 9;

			if ( z[0] == false ) {
				// set color = Black
				for ( int o = 0; o < 6; o++ ) {
					verticesAux[n] = 0.0f;
					verticesAux[n + 1] = 0.0f;
					verticesAux[n + 2] = 0.0f;
					n += 11;
					n2 += 11;
				}
			} else { // goes towards z-
				caratz = textures[0];
				float addx = 0.0f, addy = 0.0f;
				if ( x[0] ) {
					addx = -0.333f;
				} else if ( x[2] ) {
					addx = 0.333f;
				}

				if ( y[0] ) {
					addy = -0.333f;
				} else if ( y[2] ) {
					addy = 0.333f;
				}
				for ( int o = 0; o < 6; o++ ) {
					verticesAux[n2] += addx;
					verticesAux[n2 + 1] += addy;
					n += 11;
					n2 += 11;
				}
			}

			if ( z[2] == false ) {
				for ( int o = 0; o < 6; o++ ) {
					verticesAux[n] = 0.0f;
					verticesAux[n + 1] = 0.0f;
					verticesAux[n + 2] = 0.0f;
					n += 11;
					n2 += 11;
				}
			} else {
				caratz = textures[1];
				float addx = 0.0f, addy = 0.0f;
				if ( x[0] ) {
					addx = -0.333f;
				} else if ( x[2] ) {
					addx = 0.333f;
				}

				if ( y[0] ) {
					addy = -0.333f;
				} else if ( y[2] ) {
					addy = 0.333f;
				}
				for ( int o = 0; o < 6; o++ ) {
					verticesAux[n2] += addx;
					verticesAux[n2 + 1] += addy;
					n += 11;
					n2 += 11;
				}
			}

			if ( x[0] == false ) {
				for ( int o = 0; o < 6; o++ ) {
					verticesAux[n] = 0.0f;
					verticesAux[n + 1] = 0.0f;
					verticesAux[n + 2] = 0.0f;
					n += 11;
					n2 += 11;
				}
			} else {
				caratx = textures[2];
				float addx = 0.0f, addy = 0.0f;
				if ( z[0] ) {
					addx = 0.333f;
				} else if ( z[2] ) {
					addx = -0.333f;
				}

				if ( y[0] ) {
					addy = 0.333f;
				} else if ( y[2] ) {
					addy = -0.333f;
				}

				for ( int o = 0; o < 6; o++ ) {
					verticesAux[n2] += addx;
					verticesAux[n2 + 1] += addy;
					n += 11;
					n2 += 11;
				}
			}

			if ( x[2] == false ) {
				for ( int o = 0; o < 6; o++ ) {
					verticesAux[n] = 0.0f;
					verticesAux[n + 1] = 0.0f;
					verticesAux[n + 2] = 0.0f;
					n += 11;
					n2 += 11;
				}
			} else {
				caratx = textures[3];
				float addx = 0.0f, addy = 0.0f;
				if ( z[0] ) {
					addx = 0.333f;
				} else if ( z[2] ) {
					addx = -0.333f;
				}

				if ( y[0] ) {
					addy = 0.333f;
				} else if ( y[2] ) {
					addy = -0.333f;
				}

				for ( int o = 0; o < 6; o++ ) {
					verticesAux[n2] += addx;
					verticesAux[n2 + 1] += addy;
					n += 11;
					n2 += 11;
				}
			}

			if ( y[0] == false ) {
				for ( int o = 0; o < 6; o++ ) {
					verticesAux[n] = 0.0f;
					verticesAux[n + 1] = 0.0f;
					verticesAux[n + 2] = 0.0f;
					n += 11;
					n2 += 11;
				}
			} else {
				caraty = textures[4];
				float addx = 0.0f, addy = 0.0f;
				if ( x[0] ) {
					addx = -0.333f;
				} else if ( x[2] ) {
					addx = 0.333f;
				}

				if ( z[0] ) {
					addy = -0.333f;
				} else if ( z[2] ) {
					addy = 0.333f;
				}

				for ( int o = 0; o < 6; o++ ) {
					verticesAux[n2] += addx;
					verticesAux[n2 + 1] += addy;
					n += 11;
					n2 += 11;
				}
			}

			if ( y[2] == false ) {
				for ( int o = 0; o < 6; o++ ) {
					verticesAux[n] = 0.0f;
					verticesAux[n + 1] = 0.0f;
					verticesAux[n + 2] = 0.0f;
					n += 11;
					n2 += 11;
				}
			} else {
				caraty = textures[3];
				float addx = 0.0f, addy = 0.0f;
				if ( x[0] ) {
					addx = -0.333f;
				} else if ( x[2] ) {
					addx = 0.333f;
				}

				if ( z[0] ) {
					addy = -0.333f;
				} else if ( z[2] ) {
					addy = 0.333f;
				}

				for ( int o = 0; o < 6; o++ ) {
					verticesAux[n2] += addx;
					verticesAux[n2 + 1] += addy;
					n += 11;
					n2 += 11;
				}
			}
			//cubePositions[i] += offset;
			for ( int j = 0; j < size; j += 11 ) {
				mat4 matrix = mat4 (1.0f);
				matrix = translate (matrix, cubePositions[i] + offset);
				vec3 transformed = vec3 (matrix * vec4 (vertices[j], vertices[j + 1], vertices[j + 2], 1.0));
				verticesAux[j] = transformed.x;
				verticesAux[j + 1] = transformed.y;
				verticesAux[j + 2] = transformed.z;
			}

			cubesAround[i].set_id (i + id);
			for ( int l = 0; l < size; ++l )
				cubesAround[i].vertices[l] = verticesAux[l];

			cubesAround[i].bufferConfigCube ();
			cubesAround[i].direccion = cubePositions[i];
			cubesAround[i].carax = (int)cubePositions[i].x;
			cubesAround[i].caray = (int)cubePositions[i].y;
			cubesAround[i].caraz = (int)cubePositions[i].z;
			if ( cubePositions[i].x != 0.0f ) {
				cubesAround[i].numero_pintados++;
			}
			if ( cubePositions[i].y != 0.0f ) {
				cubesAround[i].numero_pintados++;
			}
			if ( cubePositions[i].z != 0.0f ) {
				cubesAround[i].numero_pintados++;
			}
			cubesAround[i].caratx = caratx;
			cubesAround[i].caraty = caraty;
			cubesAround[i].caratz = caratz;
			cubesAround[i].offset = offset;
		}
	}

	void explotar_cube () {
		if ( explode ) {
			for ( int i = 0; i < 27; ++i ) {
				cubesAround[i].explotar (direccion);
			}
			explode = false;
		}
	}

	void desexplotar_cube () {
		if ( deExplode ) {
			for ( int i = 0; i < 27; ++i ) {
				cubesAround[i].desexplotar (direccion);
			}
			deExplode = false;
		}
	}

	// RIGHT FACE
	void mover_giro_RH () { /// R
		if ( teclaMov[0] ) {
			angleMovement += 5;
			if ( all || allSolve ) {
				if ( carax == 1 )
					for ( int i = 0; i < 27; ++i )
						cubesAround[i].move_x_negative (angleMovement, true);

				if ( angleMovement == 90 && carax == 1 ) {
					mat4 matrix = mat4 (1.0f);
					matrix = rotate (matrix, radians (90.0f), vec3 (-1.0f, 0.0f, 0.0f));
					direccion = vec3 (matrix * vec4 (direccion, 1.0f));
					offset = vec3 (matrix * vec4 (offset, 1.0f));
					actualizar_giros ();
				}
			} else {
				for ( int i = 0; i < 27; ++i )
					if ( cubesAround[i].carax == 1 )
						cubesAround[i].move_x_negative (angleMovement);
				if ( angleMovement == 90 )
					moves += "R ";
			}
		}
	}
	void mover_giro_RA () { /// R'
		if ( teclaMov[1] ) {
			angleMovement += 5;
			if ( all || allSolve ) {
				if ( carax == 1 )
					for ( int i = 0; i < 27; ++i )
						cubesAround[i].move_x_positive (angleMovement, true);

				if ( angleMovement == 90 && carax == 1 ) {
					mat4 matrix = mat4 (1.0f);
					matrix = rotate (matrix, radians (90.0f), vec3 (1.0f, 0.0f, 0.0f));
					direccion = vec3 (matrix * vec4 (direccion, 1.0f));
					offset = vec3 (matrix * vec4 (offset, 1.0f));
					actualizar_giros ();
				}
			} else {
				for ( int i = 0; i < 27; ++i )
					if ( cubesAround[i].carax == 1 )
						cubesAround[i].move_x_positive (angleMovement);
				if ( angleMovement == 90 )
					moves += "R ";
			}
		}
	}
	// LEFT FACE
	void mover_giro_LH () { /// L
		if ( teclaMov[2] ) {
			angleMovement += 5;
			if ( all || allSolve ) {
				if ( carax == -1 )
					for ( int i = 0; i < 27; ++i )
						cubesAround[i].move_x_positive (angleMovement, true);
				if ( angleMovement == 90 && carax == -1 ) {
					mat4 matrix = mat4 (1.0f);
					matrix = rotate (matrix, radians (90.0f), vec3 (1.0f, 0.0f, 0.0f));
					direccion = vec3 (matrix * vec4 (direccion, 1.0));
					offset = vec3 (matrix * vec4 (offset, 1.0f));
					actualizar_giros ();
				}
			} else {
				for ( int i = 0; i < 27; ++i )
					if ( cubesAround[i].carax == -1 )
						cubesAround[i].move_x_positive (angleMovement);
				if ( angleMovement == 90 )
					moves += "L ";
			}
		}
	}
	void mover_giro_LA () { /// L'
		if ( teclaMov[3] ) {
			angleMovement += 5;
			if ( all || allSolve ) {
				if ( carax == -1 )
					for ( int i = 0; i < 27; ++i )
						cubesAround[i].move_x_negative (angleMovement, true);
				if ( angleMovement == 90 && carax == -1 ) {
					mat4 matrix = mat4 (1.0f);
					matrix = rotate (matrix, radians (90.0f), vec3 (-1.0f, 0.0f, 0.0f));
					direccion = vec3 (matrix * vec4 (direccion, 1.0));
					offset = vec3 (matrix * vec4 (offset, 1.0f));
					actualizar_giros ();
				}
			} else {
				for ( int i = 0; i < 27; ++i )
					if ( cubesAround[i].carax == -1 )
						cubesAround[i].move_x_negative (angleMovement);
				if ( angleMovement == 90 )
					moves += "L ";
			}
		}
	}
	// BACK FACE
	void mover_giro_BH () { /// B
		if ( teclaMov[4] ) {
			angleMovement += 5;
			if ( all || allSolve ) {
				if ( caraz == -1 )
					for ( int i = 0; i < 27; ++i )
						cubesAround[i].move_z_positive (angleMovement, true);
				if ( angleMovement == 90 && caraz == -1 ) {
					mat4 matrix = mat4 (1.0f);
					matrix = rotate (matrix, radians (90.0f), vec3 (0.0f, 0.0f, 1.0f));
					direccion = vec3 (matrix * vec4 (direccion, 1.0f));
					offset = vec3 (matrix * vec4 (offset, 1.0f));
					actualizar_giros ();
				}
			} else {
				for ( int i = 0; i < 27; ++i )
					if ( cubesAround[i].caraz == -1 )
						cubesAround[i].move_z_positive (angleMovement);
				if ( angleMovement == 90 )
					moves += "B ";
			}
		}
	}
	void mover_giro_BA () { /// B'
		if ( teclaMov[5] ) {
			angleMovement += 5;
			if ( all || allSolve ) {
				if ( caraz == -1 )
					for ( int i = 0; i < 27; ++i )
						cubesAround[i].move_z_negative (angleMovement, true);
				if ( angleMovement == 90 && caraz == -1 ) {
					mat4 matrix = mat4 (1.0f);
					matrix = rotate (matrix, radians (90.0f), vec3 (0.0f, 0.0f, -1.0f));
					direccion = vec3 (matrix * vec4 (direccion, 1.0f));
					offset = vec3 (matrix * vec4 (offset, 1.0f));
					actualizar_giros ();
				}
			} else {
				for ( int i = 0; i < 27; ++i )
					if ( cubesAround[i].caraz == -1 )
						cubesAround[i].move_z_negative (angleMovement);
				if ( angleMovement == 90 )
					moves += "B ";
			}
		}
	}
	// FRONT FACE
	void mover_giro_FH () { /// F
		if ( teclaMov[6] ) {
			angleMovement += 5;
			if ( all || allSolve ) {
				if ( caraz == 1 )
					for ( int i = 0; i < 27; ++i )
						cubesAround[i].move_z_negative (angleMovement, true);
				if ( angleMovement == 90 && caraz == 1 ) {
					mat4 matrix = mat4 (1.0f);
					matrix = rotate (matrix, radians (90.0f), vec3 (0.0f, 0.0f, -1.0f));
					direccion = vec3 (matrix * vec4 (direccion, 1.0));
					offset = vec3 (matrix * vec4 (offset, 1.0f));
					actualizar_giros ();
				}
			} else {
				for ( int i = 0; i < 27; ++i )
					if ( cubesAround[i].caraz == 1 )
						cubesAround[i].move_z_negative (angleMovement);
				if ( angleMovement == 90 )
					moves += "F ";
			}
		}
	}
	void mover_giro_FA () { /// F'
		if ( teclaMov[7] ) {
			angleMovement += 5;
			if ( all || allSolve ) {
				if ( caraz == 1 )
					for ( int i = 0; i < 27; ++i )
						cubesAround[i].move_z_positive (angleMovement, true);
				if ( angleMovement == 90 && caraz == 1 ) {
					mat4 matrix = mat4 (1.0f);
					matrix = rotate (matrix, radians (90.0f), vec3 (0.0f, 0.0f, 1.0f));
					direccion = vec3 (matrix * vec4 (direccion, 1.0));
					offset = vec3 (matrix * vec4 (offset, 1.0f));
					actualizar_giros ();
				}
			} else {
				for ( int i = 0; i < 27; ++i )
					if ( cubesAround[i].caraz == 1 )
						cubesAround[i].move_z_positive (angleMovement);
				if ( angleMovement == 90 )
					moves += "F ";
			}
		}
	}
	// UP FACE
	void mover_giro_UH () { /// U
		if ( teclaMov[8] ) {
			angleMovement += 5;
			if ( all || allSolve ) {
				if ( caray == 1 )
					for ( int i = 0; i < 27; ++i )
						cubesAround[i].move_y_negative (angleMovement, true);
				if ( angleMovement == 90 && caray == 1 ) {
					mat4 matrix = mat4 (1.0f);
					matrix = rotate (matrix, radians (90.0f), vec3 (0.0f, -1.0f, 0.0f));
					direccion = vec3 (matrix * vec4 (direccion, 1.0));
					offset = vec3 (matrix * vec4 (offset, 1.0f));
					actualizar_giros ();
				}
			} else {
				for ( int i = 0; i < 27; ++i )
					if ( cubesAround[i].caray == 1 )
						cubesAround[i].move_y_negative (angleMovement);
				if ( angleMovement == 90 )
					moves += "U ";
			}
		}
	}
	void mover_giro_UA () { /// U'
		if ( teclaMov[9] ) {
			angleMovement += 5;
			if ( all || allSolve ) {
				if ( caray == 1 )
					for ( int i = 0; i < 27; ++i )
						cubesAround[i].move_y_positive (angleMovement, true);
				if ( angleMovement == 90 && caray == 1 ) {
					mat4 matrix = mat4 (1.0f);
					matrix = rotate (matrix, radians (90.0f), vec3 (0.0f, 1.0f, 0.0f));
					direccion = vec3 (matrix * vec4 (direccion, 1.0));
					offset = vec3 (matrix * vec4 (offset, 1.0f));
					actualizar_giros ();
				}
			} else {
				for ( int i = 0; i < 27; ++i )
					if ( cubesAround[i].caray == 1 )
						cubesAround[i].move_y_positive (angleMovement);
				if ( angleMovement == 90 )
					moves += "U ";
			}
		}
	}
	// DOWN FACE
	void mover_giro_DH () { /// D
		if ( teclaMov[10] ) {
			angleMovement += 5;
			if ( all || allSolve ) {
				if ( caray == -1 )
					for ( int i = 0; i < 27; ++i )
						cubesAround[i].move_y_positive (angleMovement, true);
				if ( angleMovement == 90 && caray == -1 ) {
					mat4 matrix = mat4 (1.0f);
					matrix = rotate (matrix, radians (90.0f), vec3 (0.0f, 1.0f, 0.0f));
					direccion = vec3 (matrix * vec4 (direccion, 1.0));
					offset = vec3 (matrix * vec4 (offset, 1.0f));
					actualizar_giros ();
				}
			} else {
				for ( int i = 0; i < 27; ++i )
					if ( cubesAround[i].caray == -1 )
						cubesAround[i].move_y_positive (angleMovement);
				if ( angleMovement == 90 )
					moves += "D ";
			}
		}
	}
	void mover_giro_DA () { /// D'
		if ( teclaMov[11] ) {
			angleMovement += 5;
			if ( all || allSolve ) {
				if ( caray == -1 )
					for ( int i = 0; i < 27; ++i )
						cubesAround[i].move_y_negative (angleMovement, true);
				if ( angleMovement == 90 && caray == -1 ) {
					mat4 matrix = mat4 (1.0f);
					matrix = rotate (matrix, radians (90.0f), vec3 (0.0f, -1.0f, 0.0f));
					direccion = vec3 (matrix * vec4 (direccion, 1.0));
					offset = vec3 (matrix * vec4 (offset, 1.0f));
					actualizar_giros ();
				}
			} else {
				for ( int i = 0; i < 27; ++i )
					if ( cubesAround[i].caray == -1 )
						cubesAround[i].move_y_negative (angleMovement);
				if ( angleMovement == 90 )
					moves += "D ";
			}
		}
	}

	void aleatorio () {
		if ( shuffle ) {
			char movesArr[] = { 'U','F','D','L','R','B' };
			for ( int i = 0; i < 30; ++i )
				movesSolver += movesArr[rand () % 6];

			shuffle = false;
			shuffle2 = true;
			solveRubik = true;
		}
	}

	int nearestFace () {
		// check nearest face
		cubo *centralFace = &cubesAround[0];
		int index = -1;
		int fila = 0;
		double distance;
		double minDistance = INT_MAX;
		for ( int i = 0; i < 6; ++i ) {
			vec3 centralVertex = vec3 (0.0f);
			for ( int j = 0; j < 6; ++j ) {
				centralVertex += vec3 (centralFace->vertices[fila], centralFace->vertices[fila + 1], centralFace->vertices[fila + 2]);
				fila += 11;
			}
			centralVertex.x /= 6;
			centralVertex.y /= 6;
			centralVertex.z /= 6;
			distance = sqrt (pow (camera.Position.x - centralVertex.x, 2) + pow (camera.Position.y - centralVertex.y, 2) + pow (camera.Position.z - centralVertex.z, 2));
			//cout << "In index " << i << " distance: " << distance << endl;
			if ( distance < minDistance ) {
				minDistance = distance;
				index = i;
			}
		}
		return index;
	}

	void draw () {
		int index = nearestFace ();
		//cout << "Nearest Face: " << index << endl;

		for ( int i = 0; i < 27; ++i )
			cubesAround[i].draw ();

		if ( (solveRubik || allSolve) && angleMovement == 0 )
			moveFace (movesSolver);

		explotar_cube ();
		desexplotar_cube ();
		aleatorio ();
		mover_giro_RH ();
		mover_giro_RA ();
		mover_giro_LH ();
		mover_giro_LA ();
		mover_giro_BH ();
		mover_giro_BA ();
		mover_giro_FH ();
		mover_giro_FA ();
		mover_giro_UH ();
		mover_giro_UA ();
		mover_giro_DH ();
		mover_giro_DA ();

		if ( angleMovement == 90 ) {
			moving = false;
			angleMovement = 0;
			all = false;
			memset (teclaMov, 0, 12);
		}
	}

	~cubeRubik () {
	}
};

class Tetaedro {
public:
	int size = 27;
	cubeRubik rubiks[27];
	bool teclaMov[12];
	bool solveAll;
	bool alreadyExploded;
	bool explode, deExplode;
	string moves;

	vec3 rubikPositions[27] = {
		/// de espacio = 0.1
		// CENTER  -0.5 a 0.5
		/// UP
		vec3 (0.0f,  4.0f,  4.0f), // center up
		vec3 (0.f,   4.0f,  0.0f), // center
		vec3 (0.0f,  4.0f, -4.0f), // center down
		vec3 (4.0f,  4.0f,  0.0f), // right mid
		vec3 (4.0f,  4.0f,  4.0f), // right up
		vec3 (4.0f,  4.0f, -4.0f), // right down
		vec3 (-4.0f, 4.0f,  0.0f), // left mid
		vec3 (-4.0f, 4.0f,  4.0f), // left up
		vec3 (-4.0f, 4.0f, -4.0f), // left down

		// MIDDLE
		vec3 (0.0f,  0.0f,  4.0f), // center up
		vec3 (0.0f,  0.0f,  0.0f), //center 
		vec3 (0.0f,  0.0f, -4.0f), // center down
		vec3 (4.0f,  0.0f,  0.0f), // right up
		vec3 (4.0f,  0.0f,  4.0f), // right mid
		vec3 (4.0f,  0.0f, -4.0f), // right down
		vec3 (-4.0f, 0.0f,  0.0f), // left up
		vec3 (-4.0f, 0.0f,  4.0f), // left mid
		vec3 (-4.0f, 0.0f, -4.0f), // left down

		// BOTTOM
		vec3 (4.0f,  -4.0f,  0.0f), // center up
		vec3 (0.0f,  -4.0f,  0.0f), // center mid
		vec3 (-4.0f, -4.0f,  0.0f), // center left
		vec3 (4.0f,  -4.0f, -4.0f), // left up  
		vec3 (0.0f,  -4.0f, -4.0f), // left mid  
		vec3 (-4.0f, -4.0f, -4.0f), // left down
		vec3 (4.0f,  -4.0f,  4.0f), // right up  
		vec3 (0.0f,  -4.0f,  4.0f), // right mid  
		vec3 (-4.0f, -4.0f,  4.0f) // right left
	};

	Tetaedro () {
		bufferConfig ();
		textureConfig ();
		string moves = "";
		solveAll = explode = deExplode = alreadyExploded = false;
		int id = 0;
		for ( int i = 0; i < size; ++i ) {
			rubiks[i].create (rubikPositions[i], id);
			id += 27;
		}
	}

	void bufferConfig () {
		//glEnable (GL_MULTISAMPLE);
		glEnable (GL_DEPTH_TEST);
		glEnable (GL_CULL_FACE);

		glGenBuffers (729, VBO);
		glGenVertexArrays (729, VAO);

		float cube[396] = {
			/// Back -z
			 0.5f,  0.5f, -0.5f,
			 0.5f, -0.5f, -0.5f,
			-0.5f, -0.5f, -0.5f,
			-0.5f, -0.5f, -0.5f,
			-0.5f,  0.5f, -0.5f,
			 0.5f,  0.5f, -0.5f,

			 /// Front +z
			 -0.5f, -0.5f,  0.5f,
			  0.5f, -0.5f,  0.5f,
			  0.5f,  0.5f,  0.5f,
			  0.5f,  0.5f,  0.5f,
			 -0.5f,  0.5f,  0.5f,
			 -0.5f, -0.5f,  0.5f,

			 /// Left -x
			 -0.5f,  0.5f,  0.5f,
			 -0.5f,  0.5f, -0.5f,
			 -0.5f, -0.5f, -0.5f,
			 -0.5f, -0.5f, -0.5f,
			 -0.5f, -0.5f,  0.5f,
			 -0.5f,  0.5f,  0.5f,

			 /// Right +x
			  0.5f, -0.5f, -0.5f,
			  0.5f,  0.5f, -0.5f,
			  0.5f,  0.5f,  0.5f,
			  0.5f,  0.5f,  0.5f,
			  0.5f, -0.5f,  0.5f,
			  0.5f, -0.5f, -0.5f,

			  /// Down -y
			 -0.5f, -0.5f, -0.5f,
			  0.5f, -0.5f, -0.5f,
			  0.5f, -0.5f,  0.5f,
			  0.5f, -0.5f,  0.5f,
			 -0.5f, -0.5f,  0.5f,
			 -0.5f, -0.5f, -0.5f,

			 /// Up +y
			  0.5f,  0.5f,  0.5f,
			  0.5f,  0.5f, -0.5f,
			 -0.5f,  0.5f, -0.5f,
			 -0.5f,  0.5f, -0.5f,
			 -0.5f,  0.5f,  0.5f,
			  0.5f,  0.5f,  0.5f
		};

		glGenBuffers (1, &lightVBO);
		glGenVertexArrays (1, &lightVAO);
		glBindVertexArray (lightVAO);

		glBindBuffer (GL_ARRAY_BUFFER, lightVBO);
		glBufferData (GL_ARRAY_BUFFER, sizeof (cube), cube, GL_STATIC_DRAW);

		glVertexAttribPointer (0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof (float), (void *)(0));
		glEnableVertexAttribArray (0);
	}

	void textureConfig () {
		int imageWidth, imageHeight, nrComponents;
		char names[5] = { 'S','U','P','C','G' };
		stbi_set_flip_vertically_on_load (true);
		for ( int i = 0; i < 5; ++i ) {
			glGenTextures (1, &textures[i]);
			glBindTexture (GL_TEXTURE_2D, textures[i]);
			// set the texture wrapping parameters
			glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	// set texture wrapping to GL_REPEAT (default wrapping method)
			glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
			// set texture filtering parameters
			glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

			string relativePath = path + "textures/" + names[i] + ".png";
			//cout << "relative Path: " << relativePath << endl;
			// load and generate the texture
			unsigned char *imageData = stbi_load (relativePath.c_str (), &imageWidth, &imageHeight, &nrComponents, 0);
			if ( imageData ) {
				GLenum format;
				if ( nrComponents == 1 )
					format = GL_RED;
				else if ( nrComponents == 3 )
					format = GL_RGB;
				else if ( nrComponents == 4 )
					format = GL_RGBA;

				glTexImage2D (GL_TEXTURE_2D, 0, format, imageWidth, imageHeight, 0, format, GL_UNSIGNED_BYTE, imageData);
				glGenerateMipmap (GL_TEXTURE_2D);
			} else
				cout << "Failed to load texture " << names[i] << endl;

			stbi_image_free (imageData);
		}
	}

	bool isSolverDone () {
		for ( int i = 0; i < 27; ++i )
			if ( rubiks[i].movesSolver != "" )
				return false;
		return true;
	}

	void draw () {
		for ( int i = 0; i < size; ++i )
			rubiks[i].draw ();
		if ( solveAll && isSolverDone () ) {
			for ( int i = 0; i < size; ++i )
				rubiks[i].draw ();
			solveTeta ();
			solveAll = false;
		}
	}

	void solveBoth () {
		solveAll = true;
		solve ();
	}

	void solveTeta () {
		cout << "Tetaedro" << endl;
		cout << "Movimientos realizados: " << moves << endl;
		string movesSolver = Rubik::solve (moves);
		for ( int i = 0; i < 27; ++i ) {
			rubiks[i].movesSolver = movesSolver;
			rubiks[i].allSolve = true;
		}
		cout << "Movimientos: " << movesSolver << endl << endl;
		moves = "";
	}

	void solve () {
		string moves;
		for ( int i = 0; i < size; ++i ) {
			cout << "Rubik " << i << endl;
			moves = rubiks[i].moves;
			rubiks[i].solveRubik = true;
			cout << "Movimientos realizados: " << rubiks[i].moves << endl;
			rubiks[i].movesSolver = Rubik::solve (moves);
			cout << "Movimientos: " << rubiks[i].movesSolver << endl << endl;
		}
	}

	void shuffleRubikCubes () {
		if ( isSolverDone () )
			alreadyExploded = false;
		if ( !alreadyExploded ) {
			alreadyExploded = true;
			for ( int i = 0; i < 27; ++i ) {
				rubiks[i].explode = true;
				rubiks[i].shuffle = true;
			}
		} else {
			for ( int i = 0; i < 27; ++i )
				rubiks[i].shuffle = true;
		}
	}

	void shuffle () {
		char movesArr[] = { 'U','F','D','L','R','B' };
		string movesSolver;
		for ( int i = 0; i < 30; ++i )
			movesSolver += movesArr[rand () % 6];
		for ( int i = 0; i < 27; ++i ) {
			rubiks[i].allSolve = true;
			rubiks[i].movesSolver = movesSolver;
		}
		moves += movesSolver;
	}

	~Tetaedro () {
	}
};

void framebuffer_size_callback (GLFWwindow *window, int w, int h) {
	glViewport (0, 0, w, h);
}

static void cursor_position_callback (GLFWwindow *window, double xpos, double ypos) {
	if ( firstMouse ) { // initially set to true
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}
	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos; // reversed since y-coordinates range from bottom to top
	lastX = xpos;
	lastY = ypos;

	camera.ProcessMouseMovement (xoffset, yoffset, rightClick);
}

Tetaedro *teta; 

void menu () {
	cout << "Keymap \n"
		<< "Rubik moves: \n"
		<< "\t - key 'R': Right face clockwise\n"
		<< "\t - key 'L': Left face clockwise\n"
		<< "\t - key 'B': Back face clockwise\n"
		<< "\t - key 'F': Front face clockwise\n"
		<< "\t - key 'U': Up face clockwise\n"
		<< "\t - key 'D': Down face clockwise\n"
		<< "\t - key 'shift + R': Right face counterclockwise\n"
		<< "\t - key 'shift + L': Left face counterclockwise\n"
		<< "\t - key 'shift + B': Back face counterclockwise\n"
		<< "\t - key 'shift + F': Front face counterclockwise\n"
		<< "\t - key 'shift + U': Up face counterclockwise\n"
		<< "\t - key 'shift + D': Down face counterclockwise\n"
		<< "Solver: \n"
		<< "\t - key 'P': Solve individual rubik\'s cubes\n"
		<< "\t - key 'shift + P': Solve individual rubik\'s cubes and then solve tetrahedron cube\n"
		<< "\t - key 'control + P': Solve tetrahedron cube\n"
		<< "Extra: \n"
		<< "\t - key 'W': Move forward \n"
		<< "\t - key 'S': Move back \n"
		<< "\t - key 'T': Shuffle each Rubik's cube \n"
		<< "\t - key 'shift + T': Shuffle general cube \n"
		<< "\t - key 'Z': Separate general cube \n"
		<< "\t - key 'X': Unite general cube \n"
		<< "\t - key 'Q': Reset structure \n"
		<< "Camera: \n"
		<< "\t - key 'scroll up': Zoom in \n"
		<< "\t - key 'scroll down': Zoom out \n"
		<< "\t - key 'hold left click': camera movement \n";
}

// check if a face is on movement
bool rotateFace (int in) {
	if ( teta->rubiks[0].moving == false ) {
		for ( int i = 0; i < 27; ++i ) {
			teta->rubiks[i].moving = true;
			//teta->teclaMov[in] = true;

			teta->rubiks[i].all = true;
			teta->rubiks[i].teclaMov[in] = true;
		}
		return true;
	}
	return false;
}

void processInput (GLFWwindow *window, int key, int scancode, int action, int mods) {
	float cameraSpeed = 15.f * deltaTime;
	if ( key == GLFW_KEY_ESCAPE && action == GLFW_PRESS )
		glfwSetWindowShouldClose (window, true);
	if ( key == GLFW_KEY_W && (action == GLFW_PRESS || action == GLFW_REPEAT) )
		camera.ProcessKeyboard (FORWARD, deltaTime);
	if ( key == GLFW_KEY_S && (action == GLFW_PRESS || action == GLFW_REPEAT) )
		camera.ProcessKeyboard (BACKWARD, deltaTime);
	if ( key == GLFW_KEY_P && action == GLFW_PRESS ) {
		if ( mods == GLFW_MOD_CONTROL ) {
			teta->solveTeta ();
		} else  if ( mods == GLFW_MOD_SHIFT ) {
			teta->solveBoth ();
		} else {
			system ("cls");
			teta->solve ();
			menu ();
		}
	}

	/// ALEATORIO
	if ( key == GLFW_KEY_T && (action == GLFW_PRESS || action == GLFW_REPEAT) )
		if ( mods == GLFW_MOD_SHIFT )
			teta->shuffle ();
		else
			teta->shuffleRubikCubes ();

	// reset
	if ( key == GLFW_KEY_Q && action == GLFW_PRESS ) {
		delete teta;
		teta = new Tetaedro ();
	}

	if ( key == GLFW_KEY_Z && (action == GLFW_PRESS || action == GLFW_REPEAT) )
		for ( int i = 0; i < 27; ++i )
			teta->rubiks[i].explode = true;
	if ( key == GLFW_KEY_X && (action == GLFW_PRESS || action == GLFW_REPEAT) )
		for ( int i = 0; i < 27; ++i )
			teta->rubiks[i].deExplode = true;

	/// RIGHT 
	if ( key == GLFW_KEY_R && (action == GLFW_PRESS || action == GLFW_REPEAT) )
		if ( mods != GLFW_MOD_SHIFT && rotateFace (0) )
			teta->moves += "R ";
		else if ( rotateFace (1) )
			teta->moves += "R\' ";

	/// LEFT
	if ( key == GLFW_KEY_L && (action == GLFW_PRESS || action == GLFW_REPEAT) )
		if ( mods != GLFW_MOD_SHIFT && rotateFace (2) )
			teta->moves += "L ";
		else if ( rotateFace (3) )
			teta->moves += "L\' ";

	/// BACK
	if ( key == GLFW_KEY_B && (action == GLFW_PRESS || action == GLFW_REPEAT) )
		if ( mods != GLFW_MOD_SHIFT && rotateFace (4) )
			teta->moves += "B ";
		else if ( rotateFace (5) )
			teta->moves += "B\'";

	/// FRONT 
	if ( key == GLFW_KEY_F && (action == GLFW_PRESS || action == GLFW_REPEAT) )
		if ( mods != GLFW_MOD_SHIFT && rotateFace (6) )
			teta->moves += "F ";
		else if ( rotateFace (7) )
			teta->moves += "F\'";

	/// UP 
	if ( key == GLFW_KEY_U && (action == GLFW_PRESS || action == GLFW_REPEAT) )
		if ( mods != GLFW_MOD_SHIFT && rotateFace (8) )
			teta->moves += "U ";
		else if ( rotateFace (9) )
			teta->moves += "U\'";
	/// DOWN
	if ( key == GLFW_KEY_D && (action == GLFW_PRESS || action == GLFW_REPEAT) )
		if ( mods != GLFW_MOD_SHIFT && rotateFace (10) )
			teta->moves += "D ";
		else if ( rotateFace (11) )
			teta->moves += "D\' ";
}

void mouseButtonCallback (GLFWwindow *window, int button, int action, int mods) {
	if ( button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS ) {
		//cout << "Right button pressed at " << lastX << "," << lastY << endl;
		rightClick = true;
	}
	if ( button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_RELEASE )
		rightClick = false;

	if ( button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS ) {
		//cout << "Left button pressed at " << lastX << "," << lastY << endl;
	}
}

void scrollCallback (GLFWwindow *window, double xOffset, double yOffset) {
	camera.ProcessMouseScroll (yOffset);
}

void displayWindow (GLFWwindow *window, Tetaedro *cubesToDraw) {
	glClearColor (0.1f, 0.1f, 0.1f, 1.0f);
	glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	float time = glfwGetTime ();
	deltaTime = time - lastFrame;
	lastFrame = time;

	/// calculate light pos
	mat4 modelLight = mat4 (1.0f);
	modelLight = translate (modelLight, vec3 (sin (time) * 15.0f, 0.0f, cos (time) * 15.0f));

	view = camera.GetViewMatrix ();
	projection = perspective (radians (camera.Zoom), (float)screenWidth / (float)screenHeight, 0.1f, 1000.0f);

	vec3 tempLightPos = vec3(modelLight * vec4 (lightPos, 1.0f));

	shader->use ();
	shader->setVec3 ("light.position", tempLightPos);
	shader->setVec3 ("viewPos", camera.Position);

	// light properties
	shader->setVec3 ("light.ambient", 0.2f, 0.2f, 0.2f);
	shader->setVec3 ("light.diffuse", 0.6f, 0.6f, 0.6f);
	shader->setVec3 ("light.specular", 1.0f, 1.0f, 1.0f);

	// material properties
	shader->setVec3 ("material.specular", 0.5f, 0.5f, 0.5f);
	shader->setFloat ("material.shininess", 64.0f);
	
	
	//shader->setVec3 ("lightPos", tempLightPos);
	//shader->setVec3 ("lightColor", 1.0f, 1.0f, 1.0f);
	//shader->setVec3 ("viewPos", camera.Position);

	shader->setMat4 ("projection", projection);
	shader->setMat4 ("view", view);
	cubesToDraw->draw ();

	lightShader->use ();
	lightShader->setMat4 ("projection", projection);
	lightShader->setMat4 ("view", view);
	lightShader->setMat4 ("model", modelLight);
	glBindVertexArray (lightVAO);
	glDrawArrays (GL_TRIANGLES, 0, 36);



	/// Reflection
	//camera.Position = vec3(-camera.Position.x, camera.Position.y,-camera.Position.z);
	//view = camera.GetViewMatrix ();
	//camera.Position = vec3 (-camera.Position.x, camera.Position.y, -camera.Position.z);
	//shader->setMat4 ("view", view);
	//glViewport (screenWidth/2-screenWidth/4, screenHeight-200, 400, 200);
	//cubesToDraw->draw ();
	glfwSwapBuffers (window); // swap buffers
	glfwPollEvents (); // checks if any events are triggered
}

int main () {
	srand (time (NULL));
	menu ();
	if ( !glfwInit () )
		return -1;
	glfwWindowHint (GLFW_CONTEXT_VERSION_MAJOR, 3); 
	glfwWindowHint (GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint (GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
	glfwWindowHint (GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

	GLFWwindow *window = glfwCreateWindow (screenWidth, screenHeight, "LearnOpenGL", NULL, NULL);
	if ( !window ) {
		cout << "Failed to create GLFW window" << endl;
		glfwTerminate (); // destroys remaining windows
		return -1;
	}

	glfwSetMouseButtonCallback (window, mouseButtonCallback);
	glfwSetScrollCallback (window, scrollCallback);
	glfwSetKeyCallback (window, processInput);
	glfwSetCursorPosCallback (window, cursor_position_callback);

	//glfwSetInputMode (window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	glfwMakeContextCurrent (window);
	glfwSetFramebufferSizeCallback (window, framebuffer_size_callback);

	if ( !gladLoadGLLoader ((GLADloadproc)glfwGetProcAddress) ) {
		cout << "Failed to initialize GLAD" << endl;
		return -1;
	}

	shader = new Shader (path + "shaders/shader.vs", path + "shaders/shader.fs", path + "shaders/shader.gs");
	lightShader = new Shader (path + "shaders/lightShader.vs", path + "shaders/lightShader.fs");

	teta = new Tetaedro ();
	while ( !glfwWindowShouldClose (window) )
		displayWindow (window, teta);

	// ------------------------------------------------------------------------
	glDeleteVertexArrays (729, VAO);
	glDeleteBuffers (729, VBO);
	glDeleteVertexArrays (1, &lightVAO);
	glDeleteBuffers (1, &lightVBO);

	glfwTerminate ();
	delete shader;
	delete teta;
	//delete cube_1;
	return 0;
}