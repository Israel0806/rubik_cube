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
using namespace std;
using namespace glm;

//string path = "/home/sebas/Documents/7 SEMESTER/COMPUTER GRAPHICS/SEMESTRE_PART2/GLFW_GLAD_GLUT_GLEW_cmake_project_Windows_Linux/GLFW_GLAD_GLUT_GLEW_cmake_project/src/rubik_cube2/";
string path = "D:/UCSP/Semestres/Semestre_VII/Computacion_Grafica/Proyecto_final/src/rubik_cube/";
int screenWidth = 800, screenHeight = 600;
unsigned int VBO[27], VAO[27];
Shader *shader;
mat4 model, view, projection;

// mouse
bool rightClick = false;


bool teclaMov[12];
bool tecla1 = false, tecla2 = false;
bool solveRubik = false, shuffle = false, shuffle2 = false;

string _moves = "";

// timing
float deltaTime = 0.0f;	// Time between current frame and last frame
float lastFrame = 0.0f; // Time of last frame

float lastX = screenWidth / 2, lastY = screenHeight / 2;
bool firstMouse = true;

//Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
Camera camera (vec3 (0.0f, 2.0f, 15.0f));

//clase cubo 
class cubo {
public:
	int size = 396;
	glm::mat4 model1 = glm::mat4 (1.0f);
	int id;
	int numero_pintados = 0;
	int carax = 2, caray = 2, caraz = 2;
	int caratx = -1, caraty = -1, caratz = -1;
	vec3 direccion;
	int textura;
	float vertices[396];
	cubo () {}
	cubo (int id_) {
		id = id_;
	}
	void set_id (int id_) {
		id = id_;
	}
	void actualizar_giros () {
		//cout << "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n";
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

	void move (vec3 giro, int angleMovement) {
		mat4 matrix;
		for ( int j = 0; j < size; j += 11 ) {
			matrix = mat4 (1.0f);
			matrix = rotate (matrix, radians (5.0f), giro);
			vec3 transformed = vec3 (matrix * vec4 (vertices[j], vertices[j + 1], vertices[j + 2], 1.0));
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
		matrix = mat4 (1.0f);
		matrix = rotate (matrix, radians (5.0f), giro);
		direccion = vec3 (matrix * vec4 (direccion, 1.0));
		if ( angleMovement == 90 )
			actualizar_giros ();
	}

	void move_x_positive (int angleMovement) {
		move (vec3 (1.0f, 0.0f, 0.0f), angleMovement);
	}

	void move_x_negative (int angleMovement) {
		move (vec3 (-1.0f, 0.0f, 0.0f), angleMovement);
	}

	void move_y_positive (int angleMovement) {
		move (vec3 (0.0f, 1.0f, 0.0f), angleMovement);
	}

	void move_y_negative (int angleMovement) {
		move (vec3 (0.0f, -1.0f, 0.0f), angleMovement);
	}

	void move_z_positive (int angleMovement) {
		move (vec3 (0.0f, 0.0f, 1.0f), angleMovement);
	}

	void move_z_negative (int angleMovement) {
		move (vec3 (0.0f, 0.0f, -1.0f), angleMovement);
	}

	void explotar () {
		model1 = translate (model1, vec3 (direccion.x * 3, direccion.y * 3, direccion.z * 3));
	}
	void desexplotar () {
		model1 = translate (model1, vec3 (direccion.x * -3, direccion.y * -3, direccion.z * -3));
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
	string moves;
	int angleMovement;
	// S = 0, U = 1, P = 2, C = 3, G = 4
	unsigned int textures[5];
	const int size = 396;
	cubo *cubesAround;
	float vertices[396] = {
		/// Orden: Back, Front, Left,  Right, Down, Up
		/// Coordenadas       // Normales	       // Color            //texture
		/// Back: Amarillo-0 -z
		-0.5f, -0.5f, -0.5f,   0.0f,  0.0f, -1.0f,   1.0f, 1.0f, 0.0f,   0.333f, 0.333f,
		 0.5f, -0.5f, -0.5f,   0.0f,  0.0f, -1.0f,   1.0f, 1.0f, 0.0f,   0.666f, 0.333f,
		 0.5f,  0.5f, -0.5f,   0.0f,  0.0f, -1.0f,   1.0f, 1.0f, 0.0f,   0.666f, 0.666f,
		 0.5f,  0.5f, -0.5f,   0.0f,  0.0f, -1.0f,   1.0f, 1.0f, 0.0f,   0.666f, 0.666f,
		-0.5f,  0.5f, -0.5f,   0.0f,  0.0f, -1.0f,   1.0f, 1.0f, 0.0f,   0.333f, 0.666f,
		-0.5f, -0.5f, -0.5f,   0.0f,  0.0f, -1.0f,   1.0f, 1.0f, 0.0f,   0.333f, 0.333f,

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
		 0.5f,  0.5f,  0.5f,   1.0f,  0.0f,  0.0f,   1.0f, 0.0f, 0.0f,   0.333f, 0.333f,
		 0.5f,  0.5f, -0.5f,   1.0f,  0.0f,  0.0f,   1.0f, 0.0f, 0.0f,   0.666f, 0.333f,
		 0.5f, -0.5f, -0.5f,   1.0f,  0.0f,  0.0f,   1.0f, 0.0f, 0.0f,   0.666f, 0.666f,
		 0.5f, -0.5f, -0.5f,   1.0f,  0.0f,  0.0f,   1.0f, 0.0f, 0.0f,   0.666f, 0.666f,
		 0.5f, -0.5f,  0.5f,   1.0f,  0.0f,  0.0f,   1.0f, 0.0f, 0.0f,   0.333f, 0.666f,
		 0.5f,  0.5f,  0.5f,   1.0f,  0.0f,  0.0f,   1.0f, 0.0f, 0.0f,   0.333f, 0.333f,

		 /// Down: Blue-4   -y
		-0.5f, -0.5f, -0.5f,   0.0f, -1.0f,  0.0f,   0.0f, 0.0f, 1.0f,   0.333f, 0.333f,
		 0.5f, -0.5f, -0.5f,   0.0f, -1.0f,  0.0f,   0.0f, 0.0f, 1.0f,   0.666f, 0.333f,
		 0.5f, -0.5f,  0.5f,   0.0f, -1.0f,  0.0f,   0.0f, 0.0f, 1.0f,   0.666f, 0.666f,
		 0.5f, -0.5f,  0.5f,   0.0f, -1.0f,  0.0f,   0.0f, 0.0f, 1.0f,   0.666f, 0.666f,
		-0.5f, -0.5f,  0.5f,   0.0f, -1.0f,  0.0f,   0.0f, 0.0f, 1.0f,   0.333f, 0.666f,
		-0.5f, -0.5f, -0.5f,   0.0f, -1.0f,  0.0f,   0.0f, 0.0f, 1.0f,   0.333f, 0.333f,

		/// Up: Verde-5     +y
		-0.5f,  0.5f, -0.5f,   0.0f,  1.0f,  0.0f,   0.0f, 1.0f, 0.0f,   0.333f, 0.333f,
		 0.5f,  0.5f, -0.5f,   0.0f,  1.0f,  0.0f,   0.0f, 1.0f, 0.0f,   0.666f, 0.333f,
		 0.5f,  0.5f,  0.5f,   0.0f,  1.0f,  0.0f,   0.0f, 1.0f, 0.0f,   0.666f, 0.666f,
		 0.5f,  0.5f,  0.5f,   0.0f,  1.0f,  0.0f,   0.0f, 1.0f, 0.0f,   0.666f, 0.666f,
		-0.5f,  0.5f,  0.5f,   0.0f,  1.0f,  0.0f,   0.0f, 1.0f, 0.0f,   0.333f, 0.666f,
		-0.5f,  0.5f, -0.5f,   0.0f,  1.0f,  0.0f,   0.0f, 1.0f, 0.0f,   0.333f, 0.333f,
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

	cubeRubik () {
		for ( int i = 0; i < 12; ++i )
			teclaMov[i] = false;
		moves = "";
		angleMovement = 0;
		bufferConfig ();
		textureConfig ();
		createCubes ();
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

	void bufferConfig () {
		glEnable (GL_MULTISAMPLE);
		glEnable (GL_DEPTH_TEST);
		glGenBuffers (27, VBO);
		glGenVertexArrays (27, VAO);
	}

	void actualizar_cubos_dir () {
		for ( int i = 0; i < 27; ++i ) {
			cubesAround[i].actualizar_giros ();
		}
	}

	void solve (string &moves) {
		//for ( int i = 0; i < moves.length (); ++i ) {
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
			solveRubik = false;
			if ( !shuffle2 )
				this->moves = "";
			else
				shuffle2 = false;
		}
	}

	string printMoves () {
		return moves;
	}

	void createCubes () {
		cubesAround = new cubo[27];
		//-1,0,1
		bool x[3];
		bool y[3];
		bool z[3];
		for ( int i = 0; i < 27; ++i ) {
			int caratx = -1, caraty = -1, caratz = -1;
			//// copy array
			for ( int k = 0; k < size; ++k ) {
				verticesAux[k] = vertices[k];
			}


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
			if ( ((int)cubePositions[i].x) == -1 ) {
				x[0] = true;
			} else if ( ((int)cubePositions[i].x) == 1 ) {
				x[2] = true;
			}

			if ( ((int)cubePositions[i].y) == -1 ) {
				y[0] = true;
			} else if ( (int)cubePositions[i].y == 1 ) {
				y[2] = true;
			}

			if ( ((int)cubePositions[i].z) == -1 ) {
				z[0] = true;
			} else if ( ((int)cubePositions[i].z) == 1 ) {
				z[2] = true;
			}

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

			for ( int j = 0; j < size; j += 11 ) {
				mat4 matrix = mat4 (1.0f);
				matrix = translate (matrix, cubePositions[i]);
				vec3 transformed = vec3 (matrix * vec4 (vertices[j], vertices[j + 1], vertices[j + 2], 1.0));
				verticesAux[j] = transformed.x;
				verticesAux[j + 1] = transformed.y;
				verticesAux[j + 2] = transformed.z;
			}

			cubesAround[i] = cubo (i);
			for ( int l = 0; l < size; ++l ) {
				cubesAround[i].vertices[l] = verticesAux[l];
			}
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
		}
	}

	void explotar_cube () {
		if ( tecla1 ) {
			for ( int i = 0; i < 27; ++i ) {
				cubesAround[i].explotar ();
			}
			tecla1 = false;
		}
	}

	void desexplotar_cube () {
		if ( tecla2 ) {
			for ( int i = 0; i < 27; ++i ) {
				cubesAround[i].desexplotar ();
			}
			tecla2 = false;
		}
	}

	// RIGHT FACE
	void mover_giro_RH () { /// R
		if ( teclaMov[0] ) {
			angleMovement += 5;
			for ( int i = 0; i < 27; ++i )
				if ( cubesAround[i].carax == 1 )
					cubesAround[i].move_x_negative (angleMovement);
			if ( angleMovement == 90 )
				moves += "R ";
		}
	}
	void mover_giro_RA () { /// R'
		if ( teclaMov[1] ) {
			angleMovement += 5;
			for ( int i = 0; i < 27; ++i )
				if ( cubesAround[i].carax == 1 )
					cubesAround[i].move_x_positive (angleMovement);
			if ( angleMovement == 90 )
				moves += "R ";
			//tecla3 = false;
		}
	}
	// LEFT FACE
	void mover_giro_LH () { /// L
		if ( teclaMov[2] ) {
			angleMovement += 5;
			for ( int i = 0; i < 27; ++i )
				if ( cubesAround[i].carax == -1 )
					cubesAround[i].move_x_positive (angleMovement);
			if ( angleMovement == 90 )
				moves += "L ";
			//tecla5 = false;
		}
	}
	void mover_giro_LA () { /// L'
		if ( teclaMov[3] ) {
			angleMovement += 5;
			for ( int i = 0; i < 27; ++i )
				if ( cubesAround[i].carax == -1 )
					cubesAround[i].move_x_negative (angleMovement);
			if ( angleMovement == 90 )
				moves += "L ";
			//tecla5 = false;
		}
	}
	// BACK FACE
	void mover_giro_BH () { /// B
		if ( teclaMov[4] ) {
			angleMovement += 5;
			for ( int i = 0; i < 27; ++i )
				if ( cubesAround[i].caraz == -1 )
					cubesAround[i].move_z_positive (angleMovement);
			if ( angleMovement == 90 )
				moves += "B ";
			//tecla6 = false;
		}
	}
	void mover_giro_BA () { /// B'
		if ( teclaMov[5] ) {
			angleMovement += 5;
			for ( int i = 0; i < 27; ++i )
				if ( cubesAround[i].caraz == -1 )
					cubesAround[i].move_z_negative (angleMovement);
			if ( angleMovement == 90 )
				moves += "B ";
			//tecla6 = false;
		}
	}
	// FRONT FACE
	void mover_giro_FH () { /// F
		if ( teclaMov[6] ) {
			angleMovement += 5;
			for ( int i = 0; i < 27; ++i )
				if ( cubesAround[i].caraz == 1 )
					cubesAround[i].move_z_negative (angleMovement);
			//cubesAround[i].move_z_positive (angleMovement);
			if ( angleMovement == 90 )
				moves += "F ";
			//tecla8 = false;
		}
	}
	void mover_giro_FA () { /// F'
		if ( teclaMov[7] ) {
			angleMovement += 5;
			for ( int i = 0; i < 27; ++i )
				if ( cubesAround[i].caraz == 1 )
					cubesAround[i].move_z_positive (angleMovement);
			//cubesAround[i].move_z_positive (angleMovement);
			if ( angleMovement == 90 )
				moves += "F ";
			//tecla8 = false;
		}
	}
	// UP FACE
	void mover_giro_UH () { /// U
		if ( teclaMov[8] ) {
			angleMovement += 5;
			for ( int i = 0; i < 27; ++i )
				if ( cubesAround[i].caray == 1 )
					cubesAround[i].move_y_negative (angleMovement);
			//cubesAround[i].move_y_positive (angleMovement);
			if ( angleMovement == 90 )
				moves += "U ";
			//tecla9 = false;
		}
	}
	void mover_giro_UA () { /// U'
		if ( teclaMov[9] ) {
			angleMovement += 5;
			for ( int i = 0; i < 27; ++i )
				if ( cubesAround[i].caray == 1 )
					cubesAround[i].move_y_positive (angleMovement);
			//cubesAround[i].move_y_positive (angleMovement);
			if ( angleMovement == 90 )
				moves += "U ";
			//tecla9 = false;
		}
	}
	// DOWN FACE
	void mover_giro_DH () { /// D
		if ( teclaMov[10] ) {
			angleMovement += 5;
			for ( int i = 0; i < 27; ++i )
				if ( cubesAround[i].caray == -1 )
					cubesAround[i].move_y_positive (angleMovement);
			if ( angleMovement == 90 )
				moves += "D ";
			//tecla11 = false;
		}
	}
	void mover_giro_DA () { /// D'
		if ( teclaMov[11] ) {
			angleMovement += 5;
			for ( int i = 0; i < 27; ++i )
				if ( cubesAround[i].caray == -1 )
					cubesAround[i].move_y_negative (angleMovement);
			if ( angleMovement == 90 )
				moves += "D ";
			//tecla11 = false;
		}
	}

	void aleatorio () {
		if ( shuffle ) {
			srand (time (NULL));
			char movesArr[] = { 'U','F','D','L','R','B' };
			for ( int i = 0; i < 30; ++i )
				_moves += movesArr[rand () % 6];

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

		if ( solveRubik && angleMovement == 0 )
			solve (_moves);


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
			angleMovement = 0;
			if ( teclaMov[0] )
				teclaMov[0] = false;
			else if ( teclaMov[1] )
				teclaMov[1] = false;
			else if ( teclaMov[2] )
				teclaMov[2] = false;
			else if ( teclaMov[3] )
				teclaMov[3] = false;
			else if ( teclaMov[4] )
				teclaMov[4] = false;
			else if ( teclaMov[5] )
				teclaMov[5] = false;
			else if ( teclaMov[6] )
				teclaMov[6] = false;
			else if ( teclaMov[7] )
				teclaMov[7] = false;
			else if ( teclaMov[8] )
				teclaMov[8] = false;
			else if ( teclaMov[9] )
				teclaMov[9] = false;
			else if ( teclaMov[10] )
				teclaMov[10] = false;
			else if ( teclaMov[11] )
				teclaMov[11] = false;

			//tecla3 = tecla4 = tecla5 = tecla6 = tecla7 = tecla8 = tecla9 = tecla10 = tecla11 = false;
		}
	}

	~cubeRubik () {
		delete[] cubesAround;
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

cubeRubik *cube_1;

void menu () {
	cout << "Keymap \n"
		<< "Rubik moves: \n"
		<< "\t - key 'R': Right face \n"
		<< "\t - key 'L': Left face \n"
		<< "\t - key 'B': Back face \n"
		<< "\t - key 'F': Front face \n"
		<< "\t - key 'U': Up face \n"
		<< "\t - key 'D': Down face \n"
		<< "Extra \n"
		<< "\t - key 'W': Move back \n"
		<< "\t - key 'S': Move forward \n"
		<< "\t - key 'T': Shuffle Rubik's cube \n"
		<< "\t - key 'Z': Separate cubes \n"
		<< "\t - key 'X': Unite cubes \n"
		<< "Camera \n"
		<< "\t - key 'scroll up': Zoom in \n"
		<< "\t - key 'scroll down': Zoom out \n"
		<< "\t - key 'hold left click': camera movement \n";
}

// check if a face is on movement
void rotateFace (int in) {
	bool can = true;
	for ( int i = 0; i < 12; ++i ) {
		if ( in != i && teclaMov[i] ) {
			can = false;
			break;
		}
	}
	if ( can )
		teclaMov[in] = true;
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
		system ("cls");
		string moves = cube_1->printMoves ();
		//cube_1->moves = "";
		cout << "Movimientos realizados: " << moves << endl;
		cout << "Solucion" << endl;
		_moves = Rubik::solve (moves);
		cout << "Movimientos: " << _moves << endl << endl;
		//cube_1->solve (moves);
		solveRubik = true;
		menu ();
	}
	// reset
	if ( key == GLFW_KEY_Q && action == GLFW_PRESS ) {
		delete cube_1;
		cube_1 = new cubeRubik ();
		solveRubik = tecla1 = tecla2 = false;
	}

	if ( key == GLFW_KEY_Z && (action == GLFW_PRESS || action == GLFW_REPEAT) )
		tecla1 = true;
	if ( key == GLFW_KEY_X && (action == GLFW_PRESS || action == GLFW_REPEAT) )
		tecla2 = true;

	/// RIGHT 
	if ( key == GLFW_KEY_R && (action == GLFW_PRESS || action == GLFW_REPEAT) )
		if ( mods != GLFW_MOD_SHIFT )
			rotateFace (0);
		else
			rotateFace (1);

	/// LEFT
	if ( key == GLFW_KEY_L && (action == GLFW_PRESS || action == GLFW_REPEAT) )
		if ( mods != GLFW_MOD_SHIFT )
			rotateFace (2);
		else
			rotateFace (3);
	/// BACK
	if ( key == GLFW_KEY_B && (action == GLFW_PRESS || action == GLFW_REPEAT) )
		if ( mods != GLFW_MOD_SHIFT )
			rotateFace (4);
		else
			rotateFace (5);
	/// FRONT 
	if ( key == GLFW_KEY_F && (action == GLFW_PRESS || action == GLFW_REPEAT) )
		if ( mods != GLFW_MOD_SHIFT )
			rotateFace (6);
		else
			rotateFace (7);
	/// UP 
	if ( key == GLFW_KEY_U && (action == GLFW_PRESS || action == GLFW_REPEAT) )
		if ( mods != GLFW_MOD_SHIFT )
			rotateFace (8);
		else
			rotateFace (9);

	/// DOWN
	if ( key == GLFW_KEY_D && (action == GLFW_PRESS || action == GLFW_REPEAT) )
		if ( mods != GLFW_MOD_SHIFT )
			rotateFace (10);
		else
			rotateFace (11);
	/// ALEATORIO
	if ( key == GLFW_KEY_T && (action == GLFW_PRESS || action == GLFW_REPEAT) )
		shuffle = true;
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

void displayWindow (GLFWwindow *window, cubeRubik *cubesToDraw) {
	glClearColor (0.1f, 0.1f, 0.1f, 1.0f);
	glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	float time = glfwGetTime ();
	deltaTime = time - lastFrame;
	lastFrame = time;

	shader->setFloat ("time", time);

	view = camera.GetViewMatrix ();
	projection = perspective (radians (camera.Zoom), (float)screenWidth / (float)screenHeight, 0.1f, 100.0f);

	shader->use ();
	shader->setMat4 ("projection", projection);
	shader->setMat4 ("view", view);
	cubesToDraw->draw ();
	glfwSwapBuffers (window); // swap buffers
	glfwPollEvents (); // checks if any events are triggered
}

int main () {
	menu ();
	//cout << "Presionando las teclas:\n h, para mover la cara derecha;\n j, para mover la cara izquierda;\n k, para mover la cara de atrás;\n l, para mover la cara de adelante;\n z, para mover la cara de arriba;\n x, para mover la cara de abajo.\n";
	//cout << "Para separa los cubitos presionar f\n para unnir os cubitos presionanr g\n para ejecutar el aleatorio presionar t\n";
	//cout << "Mantener click derecho para movilizar la camara\n";
	if ( !glfwInit () )
		return -1;
	// para el manejo de la ventana
	glfwWindowHint (GLFW_CONTEXT_VERSION_MAJOR, 3); // mayor and minor version for openGL 3
	glfwWindowHint (GLFW_CONTEXT_VERSION_MINOR, 3);
	// to use core_profile for access to subsets of features 
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

	shader = new Shader (path + "shaders/shader.vs", path + "shaders/shader.fs");

	cube_1 = new cubeRubik ();
	while ( !glfwWindowShouldClose (window) )
		displayWindow (window, cube_1);

	// ------------------------------------------------------------------------
	glDeleteVertexArrays (27, VAO);
	glDeleteBuffers (27, VBO);
	glfwTerminate ();
	delete shader;
	delete cube_1;
	return 0;
}