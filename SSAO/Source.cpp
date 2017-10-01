#include <GL/glew.h>
#include <GL/glut.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <cmath>
#include <cstdlib>
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <random>

using glm::vec3;

/*
 * TRABAJO FINAL PATG
 *
 * SSAO
 *
 * EMILIO JOSÉ MONTEJANO MORA
 *
 */

const int MAXKERNELSIZE = 128; //Tamaño máximo del vector kernel

void load_obj(const char*, std::vector<glm::vec4> &, std::vector<glm::vec3> &, std::vector<GLushort> &);
//Buddha, bunny, dragon
int initObj1(std::string);
int initObj2(std::string);
int initObj3(std::string);
void drawObj1();
void drawObj2();
void drawObj3();

void MensajeInicio();

void CreatePrograms();
void FillUniformVariables();
void CreateBuffers();
void GenerateSampleKernelAndNoiseTexture();
void GenerateSampleKernel();
void GenerateNoiseTexture();
void CreateGBuffer();
void CreateDepthBuffer();
void CreateSSAOBuffer();
void CreateBlurBuffer();

void loadSource(GLuint &shaderID, std::string name);
void printCompileInfoLog(GLuint shadID);
void printLinkInfoLog(GLuint programID);
void validateProgram(GLuint programID);

bool init();

void display();
void resize(int, int);
void idle();
void keyboard(unsigned char, int, int);
void specialKeyboard(int, int, int);
void mouse(int, int, int, int);
void mouseMotion(int, int);

bool fullscreen = false;
bool mouseDown = false;
bool animation = false;

float xrot = 0.0f;
float yrot = 0.0f;
float xdiff = 0.0f;
float ydiff = 0.0f;

int g_Width = 712;                          // Ancho inicial de la ventana
int g_Height = 712;                         // Altura incial de la ventana

void ChangeProperty(bool increase);
void ChangeSSOORadius(bool increase);
void ChangeSSOOBias(bool increase);
void ChangeSSOOPower(bool increase);
void ChangeSSOOKernelSize(bool increase);
void ChangeSSOONoiseTextureSize(bool increase);

//Genera floats aleatorios de 0.0 a 1.0
std::uniform_real_distribution<GLfloat> randomFloats(0.0, 1.0);
std::default_random_engine generator;

//Buddha, bunny, dragon
GLuint objVAOHandle1, objVAOHandle2, objVAOHandle3;

GLuint programID1, programID2, programID3, programID4;

GLuint gBuffer;
GLuint ssaoFBO;
GLuint noiseTexture;
GLuint ssaoColorBuffer;
GLuint ssaoColorBufferBlur;
GLuint ssaoBlurFBO;
std::vector<glm::vec3> ssaoKernel;

GLuint gPositionTex, gNormalTex, gAlbedoTex;

GLuint locUniformMVP, locUniformMV, locUniformNM, locUniformSSAOKernel[MAXKERNELSIZE], locUniformP, locUniformDrawMode1,
locUniformDrawMode2, locUniformLightPosition, locUniformLightColor, locUniformLightLinear, locUniformLightQuadratic,
locUniformSSAORadius, locUniformSSAOBias, locUniformSSAOPower, locUniformSSAOKernelSize, locUniformSSAONoiseTexture,
locUniformSSAONoiseTextureSize;

//PROPIEDES
//Modo de dibujo, escogemos que queremos mostrar
GLuint draw_mode = 1;

int property_mode = 0;
float SSAORadius = 0.5f;
float SSAOBias = 0.025f;
float SSAOPower = 1.0f;
int SSAOKernelSize = 64;
int SSAONoiseTextureSize = 4; //Tamaño de la textura de ruido (cada lado)
const float SSAORadiusMax = 15.0f, SSAORadiusMin = 1.0f;
const float SSAOBiasMax = 1.0f, SSAOBiasMin = 0.001f;
const float SSAOPowerMax = 5.0f, SSAOPowerMin = 1.0f;
const int SSAOKernelSizeMin = 8;
const int SSAONoiseTextureSizeMax = 16, SSAONoiseTextureSizeMin = 1;

int numVertObj1, numVertObj2, numVertObj3;

// BEGIN: Soporte para modelos OBJ /////////////////////////////////////////////////////////////////////////////////

std::vector<GLfloat> obj_vertices;
std::vector<GLfloat> obj_normals;
std::vector<GLushort> obj_elements;

void load_obj(const char* filename, std::vector<GLfloat> &vertices, std::vector<GLfloat> &normals, std::vector<GLushort> &elements) {
	std::ifstream in(filename, std::ios::in);
	if (!in) { std::cerr << "Cannot open " << filename << std::endl; std::system("pause"); exit(1); }

	vertices.clear();
	normals.clear();
	elements.clear();

	std::string line;
	while (getline(in, line)) {
		if (line.substr(0, 2) == "v ") {
			std::istringstream s(line.substr(2));
			GLfloat x, y, z; s >> x; s >> y; s >> z;
			vertices.push_back(x);
			vertices.push_back(y);
			vertices.push_back(z);
		}
		else if (line.substr(0, 2) == "f ") {
			std::istringstream s(line.substr(2));
			GLushort a, b, c;
			s >> a; s >> b; s >> c;
			a--; b--; c--;
			elements.push_back(a); elements.push_back(b); elements.push_back(c);
		}
		else if (line[0] == '#') { /* ignoring this line */ }
		else { /* ignoring this line */ }
	}

	normals.resize(vertices.size(), 0.0f);
	for (int i = 0; i < elements.size(); i += 3) {
		GLushort ia = elements[i];
		GLushort ib = elements[i + 1];
		GLushort ic = elements[i + 2];
		glm::vec3 normal = glm::normalize(glm::cross(
			glm::vec3(vertices[ib * 3 + 0], vertices[ib * 3 + 1], vertices[ib * 3 + 2]) - glm::vec3(vertices[ia * 3 + 0], vertices[ia * 3 + 1], vertices[ia * 3 + 2]),
			glm::vec3(vertices[ic * 3 + 0], vertices[ic * 3 + 1], vertices[ic * 3 + 2]) - glm::vec3(vertices[ia * 3 + 0], vertices[ia * 3 + 1], vertices[ia * 3 + 2])));
		normals[ia * 3 + 0] = normals[ib * 3 + 0] = normals[ic * 3 + 0] = normal.x;
		normals[ia * 3 + 1] = normals[ib * 3 + 1] = normals[ic * 3 + 1] = normal.y;
		normals[ia * 3 + 2] = normals[ib * 3 + 2] = normals[ic * 3 + 2] = normal.z;
	}
}

//Buddha
int initObj1(std::string file)
{
	load_obj(file.c_str(), obj_vertices, obj_normals, obj_elements);

	glGenVertexArrays(1, &objVAOHandle1);
	glBindVertexArray(objVAOHandle1);

	unsigned int handle[3];
	glGenBuffers(3, handle);

	glBindBuffer(GL_ARRAY_BUFFER, handle[0]);
	glBufferData(GL_ARRAY_BUFFER, obj_vertices.size() * sizeof(GLfloat), obj_vertices.data(), GL_STATIC_DRAW); // Datos de la posición de los vértices
	GLuint loc1 = glGetAttribLocation(programID1, "position");
	glEnableVertexAttribArray(loc1); // Vertex position
	glVertexAttribPointer(loc1, 3, GL_FLOAT, GL_FALSE, 0, (GLubyte *)NULL + 0);

	glBindBuffer(GL_ARRAY_BUFFER, handle[1]);
	glBufferData(GL_ARRAY_BUFFER, obj_normals.size() * sizeof(GLfloat), obj_normals.data(), GL_STATIC_DRAW); // Datos de las normales de los vértices
	GLuint loc2 = glGetAttribLocation(programID1, "normal");
	glEnableVertexAttribArray(loc2); // Vertex normal
	glVertexAttribPointer(loc2, 3, GL_FLOAT, GL_FALSE, 0, (GLubyte *)NULL + 0);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, handle[2]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, obj_elements.size() * sizeof(GLushort), obj_elements.data(), GL_STATIC_DRAW); // Array de índices

	glBindVertexArray(0);

	return obj_elements.size();
}

//bunny
int initObj2(std::string file)
{
	load_obj(file.c_str(), obj_vertices, obj_normals, obj_elements);

	glGenVertexArrays(1, &objVAOHandle2);
	glBindVertexArray(objVAOHandle2);

	unsigned int handle[3];
	glGenBuffers(3, handle);

	glBindBuffer(GL_ARRAY_BUFFER, handle[0]);
	glBufferData(GL_ARRAY_BUFFER, obj_vertices.size() * sizeof(GLfloat), obj_vertices.data(), GL_STATIC_DRAW); // Datos de la posición de los vértices
	GLuint loc1 = glGetAttribLocation(programID1, "position");
	glEnableVertexAttribArray(loc1); // Vertex position
	glVertexAttribPointer(loc1, 3, GL_FLOAT, GL_FALSE, 0, (GLubyte *)NULL + 0);

	glBindBuffer(GL_ARRAY_BUFFER, handle[1]);
	glBufferData(GL_ARRAY_BUFFER, obj_normals.size() * sizeof(GLfloat), obj_normals.data(), GL_STATIC_DRAW); // Datos de las normales de los vértices
	GLuint loc2 = glGetAttribLocation(programID1, "normal");
	glEnableVertexAttribArray(loc2); // Vertex normal
	glVertexAttribPointer(loc2, 3, GL_FLOAT, GL_FALSE, 0, (GLubyte *)NULL + 0);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, handle[2]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, obj_elements.size() * sizeof(GLushort), obj_elements.data(), GL_STATIC_DRAW); // Array de índices

	glBindVertexArray(0);

	return obj_elements.size();
}

//dragon
int initObj3(std::string file)
{
	load_obj(file.c_str(), obj_vertices, obj_normals, obj_elements);

	glGenVertexArrays(1, &objVAOHandle3);
	glBindVertexArray(objVAOHandle3);

	unsigned int handle[3];
	glGenBuffers(3, handle);

	glBindBuffer(GL_ARRAY_BUFFER, handle[0]);
	glBufferData(GL_ARRAY_BUFFER, obj_vertices.size() * sizeof(GLfloat), obj_vertices.data(), GL_STATIC_DRAW); // Datos de la posición de los vértices
	GLuint loc1 = glGetAttribLocation(programID1, "position");
	glEnableVertexAttribArray(loc1); // Vertex position
	glVertexAttribPointer(loc1, 3, GL_FLOAT, GL_FALSE, 0, (GLubyte *)NULL + 0);

	glBindBuffer(GL_ARRAY_BUFFER, handle[1]);
	glBufferData(GL_ARRAY_BUFFER, obj_normals.size() * sizeof(GLfloat), obj_normals.data(), GL_STATIC_DRAW); // Datos de las normales de los vértices
	GLuint loc2 = glGetAttribLocation(programID1, "normal");
	glEnableVertexAttribArray(loc2); // Vertex normal
	glVertexAttribPointer(loc2, 3, GL_FLOAT, GL_FALSE, 0, (GLubyte *)NULL + 0);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, handle[2]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, obj_elements.size() * sizeof(GLushort), obj_elements.data(), GL_STATIC_DRAW); // Array de índices

	glBindVertexArray(0);

	return obj_elements.size();
}

//Buddha
void drawObj1()  {
	glBindVertexArray(objVAOHandle1);
	glDrawElements(GL_TRIANGLES, numVertObj1, GL_UNSIGNED_SHORT, ((GLubyte *)NULL + (0)));
	glBindVertexArray(0);
}

//bunny
void drawObj2()  {
	glBindVertexArray(objVAOHandle2);
	glDrawElements(GL_TRIANGLES, numVertObj2, GL_UNSIGNED_SHORT, ((GLubyte *)NULL + (0)));
	glBindVertexArray(0);
}

//dragon
void drawObj3()  {
	glBindVertexArray(objVAOHandle3);
	glDrawElements(GL_TRIANGLES, numVertObj3, GL_UNSIGNED_SHORT, ((GLubyte *)NULL + (0)));
	glBindVertexArray(0);
}

// END:   Soporte para modelos OBJ /////////////////////////////////////////////////////////////////////////////////

// BEGIN: Carga shaders ////////////////////////////////////////////////////////////////////////////////////////////

void loadSource(GLuint &shaderID, std::string name)
{
	std::ifstream f(name.c_str());
	if (!f.is_open())
	{
		std::cerr << "File not found " << name.c_str() << std::endl;
		system("pause");
		exit(EXIT_FAILURE);
	}

	// now read in the data
	std::string *source;
	source = new std::string(std::istreambuf_iterator<char>(f),
		std::istreambuf_iterator<char>());
	f.close();

	// add a null to the string
	*source += "\0";
	const GLchar * data = source->c_str();
	glShaderSource(shaderID, 1, &data, NULL);
	delete source;
}

void printCompileInfoLog(GLuint shadID)
{
	GLint compiled;
	glGetShaderiv(shadID, GL_COMPILE_STATUS, &compiled);
	if (compiled == GL_FALSE)
	{
		GLint infoLength = 0;
		glGetShaderiv(shadID, GL_INFO_LOG_LENGTH, &infoLength);

		GLchar *infoLog = new GLchar[infoLength];
		GLint chsWritten = 0;
		glGetShaderInfoLog(shadID, infoLength, &chsWritten, infoLog);

		std::cerr << "Shader compiling failed:" << infoLog << std::endl;
		system("pause");
		delete[] infoLog;

		exit(EXIT_FAILURE);
	}
}

void printLinkInfoLog(GLuint programID)
{
	GLint linked;
	glGetProgramiv(programID, GL_LINK_STATUS, &linked);
	if (!linked)
	{
		GLint infoLength = 0;
		glGetProgramiv(programID, GL_INFO_LOG_LENGTH, &infoLength);

		GLchar *infoLog = new GLchar[infoLength];
		GLint chsWritten = 0;
		glGetProgramInfoLog(programID, infoLength, &chsWritten, infoLog);

		std::cerr << "Shader linking failed:" << infoLog << std::endl;
		system("pause");
		delete[] infoLog;

		exit(EXIT_FAILURE);
	}
}

void validateProgram(GLuint programID)
{
	GLint status;
	glValidateProgram(programID);
	glGetProgramiv(programID, GL_VALIDATE_STATUS, &status);

	if (status == GL_FALSE)
	{
		GLint infoLength = 0;
		glGetProgramiv(programID, GL_INFO_LOG_LENGTH, &infoLength);

		if (infoLength > 0)
		{
			GLchar *infoLog = new GLchar[infoLength];
			GLint chsWritten = 0;
			glGetProgramInfoLog(programID, infoLength, &chsWritten, infoLog);
			std::cerr << "Program validating failed:" << infoLog << std::endl;
			system("pause");
			delete[] infoLog;

			exit(EXIT_FAILURE);
		}
	}
}

// END:   Carga shaders ////////////////////////////////////////////////////////////////////////////////////////////


// BEGIN: Funciones de dibujo ////////////////////////////////////////////////////////////////////////////////////

// Renderiza un cubo de 1x1
GLuint cubeVAO = 0;
GLuint cubeVBO = 0;
void RenderCube()
{
	//Inicializamos el VAO si no lo hemos hecho antes
	if (cubeVAO == 0)
	{
		GLfloat vertices[] = {
			// Cara atrás
			-0.5f, -0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, 
			0.5f, 0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 1.0f, 1.0f, 
			0.5f, -0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f,         
			0.5f, 0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 1.0f, 1.0f,  
			-0.5f, -0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, 
			-0.5f, 0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f,
			// Cara delante
			-0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 
			0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f,  
			0.5f, 0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f,  
			0.5f, 0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 
			-0.5f, 0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 
			-0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
			// Cara izquierda
			-0.5f, 0.5f, 0.5f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 
			-0.5f, 0.5f, -0.5f, -1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 
			-0.5f, -0.5f, -0.5f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f,  
			-0.5f, -0.5f, -0.5f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 
			-0.5f, -0.5f, 0.5f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f,
			-0.5f, 0.5f, 0.5f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
			// Cara derecha
			0.5f, 0.5f, 0.5f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 
			0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 
			0.5f, 0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f,         
			0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f,  
			0.5f, 0.5f, 0.5f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f,  
			0.5f, -0.5f, 0.5f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f,      
			// Cara abajo
			-0.5f, -0.5f, -0.5f, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f, 
			0.5f, -0.5f, -0.5f, 0.0f, -1.0f, 0.0f, 1.0f, 1.0f, 
			0.5f, -0.5f, 0.5f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f,
			0.5f, -0.5f, 0.5f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f, 
			-0.5f, -0.5f, 0.5f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f, 
			-0.5f, -0.5f, -0.5f, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f, 
			// Cara arriba
			-0.5f, 0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f,
			0.5f, 0.5f, 0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 
			0.5f, 0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f,    
			0.5f, 0.5f, 0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 
			-0.5f, 0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f,
			-0.5f, 0.5f, 0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f        
		};
		glGenVertexArrays(1, &cubeVAO);
		glGenBuffers(1, &cubeVBO);
		
		glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
		
		glBindVertexArray(cubeVAO);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(6 * sizeof(GLfloat)));
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
	}
	
	glBindVertexArray(cubeVAO);
	glDrawArrays(GL_TRIANGLES, 0, 36);
	glBindVertexArray(0);
}

//Renderiza un quad de 1x1,
//usado para el framebuffer (color) y los efectos de post-procesado
GLuint quadVAO = 0;
GLuint quadVBO;
void RenderQuad()
{
	if (quadVAO == 0)
	{
		GLfloat quadVertices[] = {
			// Posicion (1,2,3) - Coordenadas de textura (4,5)
			-1.0f, 1.0f, 0.0f, 0.0f, 1.0f,
			-1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
			1.0f, 1.0f, 0.0f, 1.0f, 1.0f,
			1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
		};
		
		glGenVertexArrays(1, &quadVAO);
		glGenBuffers(1, &quadVBO);
		glBindVertexArray(quadVAO);
		glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
	}
	glBindVertexArray(quadVAO);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glBindVertexArray(0);
}

// END: Funciones de dibujo ////////////////////////////////////////////////////////////////////////////////////

int main(int argc, char *argv[])
{
	glutInit(&argc, argv);
	glutInitWindowPosition(50, 50);
	glutInitWindowSize(g_Width, g_Height);
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
	glutCreateWindow("SSAO - Emilio Montejano Mora");
	GLenum err = glewInit();
	if (GLEW_OK != err)
	{
		/* Problem: glewInit failed, something is seriously wrong. */
		std::cerr << "Error: " << glewGetErrorString(err) << std::endl;
		system("pause");
		exit(-1);
	}
	init();

	glutDisplayFunc(display);
	glutKeyboardFunc(keyboard);
	glutSpecialFunc(specialKeyboard);
	glutMouseFunc(mouse);
	glutMotionFunc(mouseMotion);
	glutReshapeFunc(resize);
	glutIdleFunc(idle);

	glutMainLoop();

	return EXIT_SUCCESS;
}

bool init()
{
	glClearColor(1.00f, 1.00f, 1.00f, 0.0f);
	glDisable(GL_BLEND);

	glewInit();
	glGetError();

	glViewport(0, 0, g_Width, g_Height);

	glEnable(GL_DEPTH_TEST);
	glClearDepth(1.0f);

	CreatePrograms();
	FillUniformVariables();

	numVertObj1 = initObj1("buddha.obj");
	numVertObj2 = initObj2("bunny.obj");
	numVertObj3 = initObj3("dragon.obj");

	CreateBuffers();
	GenerateSampleKernelAndNoiseTexture();

	std::cout << "Inicio finalizado ..." << std::endl;

	MensajeInicio();

	return true;
}

void MensajeInicio()
{
	std::cout << "Controles: " << std::endl;
	std::cout << "Pulsa 0 para ver la escena iluminada sin SSAO y blur" << std::endl;
	std::cout << "Pulsa 1 para ver la escena finalizada con SSAO, blur e iluminacion" << std::endl;
	std::cout << "Pulsa 2 para ver el buffer de posicion" << std::endl;
	std::cout << "Pulsa 3 para ver el buffer de normales" << std::endl;
	std::cout << "Pulsa 4 para ver el buffer de albedo (color difuso)" << std::endl;
	std::cout << "Pulsa 5 para ver el buffer SSAO sin blur" << std::endl;
	std::cout << "Pulsa 6 para ver el buffer SSAO con blur" << std::endl;

	std::cout << "Pulsa la tecla para seleccionar el parametro a cambiar" << std::endl;
	std::cout << "Pulsa '+' ó 'Z' para subir el parametro seleccionado" << std::endl;
	std::cout << "Pulsa '-' ó 'X' para bajar el parametro seleccionado" << std::endl;

	std::cout << "Lista de parametros: " << std::endl;
	std::cout << "Pulsa 'R' para cambiar el radio de la SSAO" << std::endl;
	std::cout << "Pulsa 'B' para cambiar el bias en la comprobacion de la profundidad de SSAO" << std::endl;
	std::cout << "Pulsa 'P' para cambiar la potencia de la SSAO" << std::endl;
	std::cout << "Pulsa 'K' para cambiar el tamaño del kernel" << std::endl;
	std::cout << "Pulsa 'N' para cambiar el tamaño de la textura de ruido" << std::endl;
	std::cout << "Pulsa 'E' para restablecer los valores predeterminados" << std::endl;

	std::cout << "Pulsa 'Q' para salir" << std::endl;
	std::cout << "Pulsa 'A' para mover la camara automaticamente" << std::endl;

	std::cout << "Radio seleccionado" << std::endl;
}

void CreatePrograms()
{
	programID1 = glCreateProgram();

	GLuint vertexShaderID1 = glCreateShader(GL_VERTEX_SHADER);
	loadSource(vertexShaderID1, "ssao_geometry.vert");
	std::cout << "Compiling vertex shader ..." << std::endl;
	glCompileShader(vertexShaderID1);
	printCompileInfoLog(vertexShaderID1);
	glAttachShader(programID1, vertexShaderID1);

	GLuint fragmentShaderID1 = glCreateShader(GL_FRAGMENT_SHADER);
	loadSource(fragmentShaderID1, "ssao_geometry.frag");
	std::cout << "Compiling fragment shader ..." << std::endl;
	glCompileShader(fragmentShaderID1);
	printCompileInfoLog(fragmentShaderID1);
	glAttachShader(programID1, fragmentShaderID1);

	glLinkProgram(programID1);
	printLinkInfoLog(programID1);
	validateProgram(programID1);
	std::cout << "Programa geometria completado ..." << std::endl;

	programID2 = glCreateProgram();

	GLuint vertexShaderID2 = glCreateShader(GL_VERTEX_SHADER);
	loadSource(vertexShaderID2, "ssao.vert");
	std::cout << "Compiling vertex shader ..." << std::endl;
	glCompileShader(vertexShaderID2);
	printCompileInfoLog(vertexShaderID2);
	glAttachShader(programID2, vertexShaderID2);

	GLuint fragmentShaderID2 = glCreateShader(GL_FRAGMENT_SHADER);
	loadSource(fragmentShaderID2, "ssao_lighting.frag");
	std::cout << "Compiling fragment shader ..." << std::endl;
	glCompileShader(fragmentShaderID2);
	printCompileInfoLog(fragmentShaderID2);
	glAttachShader(programID2, fragmentShaderID2);

	glLinkProgram(programID2);
	printLinkInfoLog(programID2);
	validateProgram(programID2);
	std::cout << "Programa iluminacion completado ..." << std::endl;

	programID3 = glCreateProgram();

	GLuint vertexShaderID3 = glCreateShader(GL_VERTEX_SHADER);
	loadSource(vertexShaderID3, "ssao.vert");
	std::cout << "Compiling vertex shader ..." << std::endl;
	glCompileShader(vertexShaderID3);
	printCompileInfoLog(vertexShaderID3);
	glAttachShader(programID3, vertexShaderID3);

	GLuint fragmentShaderID3 = glCreateShader(GL_FRAGMENT_SHADER);
	loadSource(fragmentShaderID3, "ssao.frag");
	std::cout << "Compiling fragment shader ..." << std::endl;
	glCompileShader(fragmentShaderID3);
	printCompileInfoLog(fragmentShaderID3);
	glAttachShader(programID3, fragmentShaderID3);

	glLinkProgram(programID3);
	printLinkInfoLog(programID3);
	validateProgram(programID3);
	std::cout << "Programa ssao completado ..." << std::endl;

	programID4 = glCreateProgram();

	GLuint vertexShaderID4 = glCreateShader(GL_VERTEX_SHADER);
	loadSource(vertexShaderID4, "ssao.vert");
	std::cout << "Compiling vertex shader ..." << std::endl;
	glCompileShader(vertexShaderID4);
	printCompileInfoLog(vertexShaderID4);
	glAttachShader(programID4, vertexShaderID4);

	GLuint fragmentShaderID4 = glCreateShader(GL_FRAGMENT_SHADER);
	loadSource(fragmentShaderID4, "ssao_blur.frag");
	std::cout << "Compiling fragment shader ..." << std::endl;
	glCompileShader(fragmentShaderID4);
	printCompileInfoLog(fragmentShaderID4);
	glAttachShader(programID4, fragmentShaderID4);

	glLinkProgram(programID4);
	printLinkInfoLog(programID4);
	validateProgram(programID4);
	std::cout << "Programa blur completado ..." << std::endl;

	//Inicializamos las variables de textura de los shaders
	glUseProgram(programID2);
	glUniform1i(glGetUniformLocation(programID2, "gPosition"), 0);
	glUniform1i(glGetUniformLocation(programID2, "gNormal"), 1);
	glUniform1i(glGetUniformLocation(programID2, "gAlbedo"), 2);
	glUniform1i(glGetUniformLocation(programID2, "ssao"), 3);
	glUseProgram(programID3);
	glUniform1i(glGetUniformLocation(programID3, "gPosition"), 0);
	glUniform1i(glGetUniformLocation(programID3, "gNormal"), 1);
	glUniform1i(glGetUniformLocation(programID3, "texNoise"), 2);
}

void FillUniformVariables()
{
	locUniformMVP = glGetUniformLocation(programID1, "uModelViewProjMatrix");
	locUniformMV = glGetUniformLocation(programID1, "uModelViewMatrix");
	locUniformNM = glGetUniformLocation(programID1, "uNormalMatrix");
	for (GLuint i = 0; i < SSAOKernelSize; ++i)
		locUniformSSAOKernel[i] = glGetUniformLocation(programID3, ("samples[" + std::to_string(i) + "]").c_str());
	locUniformP = glGetUniformLocation(programID3, "projection");
	locUniformDrawMode1 = glGetUniformLocation(programID4, "draw_mode");
	locUniformDrawMode2 = glGetUniformLocation(programID2, "draw_mode");
	locUniformLightPosition = glGetUniformLocation(programID2, "light.Position");
	locUniformLightColor = glGetUniformLocation(programID2, "light.Color");
	locUniformLightLinear = glGetUniformLocation(programID2, "light.Linear");
	locUniformLightQuadratic = glGetUniformLocation(programID2, "light.Quadratic");
	
	locUniformSSAORadius = glGetUniformLocation(programID3, "radius");
	glUniform1f(locUniformSSAORadius, SSAORadius);

	locUniformSSAOBias = glGetUniformLocation(programID3, "bias");
	glUniform1f(locUniformSSAOBias, SSAOBias);

	locUniformSSAOPower = glGetUniformLocation(programID3, "power");
	glUniform1f(locUniformSSAOPower, SSAOPower);

	locUniformSSAOKernelSize = glGetUniformLocation(programID3, "kernelSize");
	glUniform1f(locUniformSSAOKernelSize, SSAOKernelSize);

	locUniformSSAONoiseTexture = glGetUniformLocation(programID3, "noiseScale");
	glUniform2f(locUniformSSAONoiseTexture, g_Width / (float)SSAONoiseTextureSize, g_Height / (float)SSAONoiseTextureSize);

	locUniformSSAONoiseTextureSize = glGetUniformLocation(programID4, "uBlurSize");
	glUniform1f(locUniformSSAONoiseTextureSize, SSAONoiseTextureSize);
}

void CreateBuffers()
{
	CreateGBuffer();
	CreateDepthBuffer();
	CreateSSAOBuffer();
	CreateBlurBuffer();
}

void CreateGBuffer()
{
	//Creamos el G-Buffer, consta de tres texturas
	//Posiciones
	//Color  
	//Normales 
	glGenFramebuffers(1, &gBuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);

	//Buffer de posición
	glGenTextures(1, &gPositionTex);
	glBindTexture(GL_TEXTURE_2D, gPositionTex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, g_Width, g_Height, 0, GL_RGB, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, gPositionTex, 0);

	//Buffer de normales
	glGenTextures(1, &gNormalTex);
	glBindTexture(GL_TEXTURE_2D, gNormalTex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, g_Width, g_Height, 0, GL_RGB, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, gNormalTex, 0);

	//Buffer de color (difuso)
	glGenTextures(1, &gAlbedoTex);
	glBindTexture(GL_TEXTURE_2D, gAlbedoTex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, g_Width, g_Height, 0, GL_RGBA, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, gAlbedoTex, 0);
	
	GLuint attachments[3] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
	glDrawBuffers(3, attachments);
}

void CreateDepthBuffer()
{
	//Creamos un renderbuffer de profundidad
	GLuint rboDepth;
	glGenRenderbuffers(1, &rboDepth);
	glBindRenderbuffer(GL_RENDERBUFFER, rboDepth);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, g_Width, g_Height);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboDepth);

	//Comprobamos que el G-Buffer se ha construido correctamente
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		std::cout << "GBuffer no completado, error" << std::endl;
}

void CreateSSAOBuffer()
{
	glGenFramebuffers(1, &ssaoFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, ssaoFBO);
	
	glGenTextures(1, &ssaoColorBuffer);
	glBindTexture(GL_TEXTURE_2D, ssaoColorBuffer);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, g_Width, g_Height, 0, GL_RGB, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ssaoColorBuffer, 0);
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		std::cout << "SSAO Framebuffer no completado, error" << std::endl;
}

void CreateBlurBuffer()
{
	glGenFramebuffers(1, &ssaoBlurFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, ssaoBlurFBO);

	glGenTextures(1, &ssaoColorBufferBlur);
	glBindTexture(GL_TEXTURE_2D, ssaoColorBufferBlur);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, g_Width, g_Height, 0, GL_RGB, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ssaoColorBufferBlur, 0);
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		std::cout << "SSAO Blur Framebuffer no completado, error" << std::endl;

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

//Genera el ssao kernel y la textura de ruido
void GenerateSampleKernelAndNoiseTexture()
{	
	GenerateSampleKernel();
	GenerateNoiseTexture();	
}

void GenerateSampleKernel()
{
	ssaoKernel.clear();

	//SSAO kernel
	for (GLuint i = 0; i < SSAOKernelSize; ++i)
	{
		glm::vec3 sample(randomFloats(generator) * 2.0 - 1.0, randomFloats(generator) * 2.0 - 1.0, randomFloats(generator));
		sample = glm::normalize(sample);
		sample *= randomFloats(generator); //escalamos las muestras para distribuirlas por el hemisferio
		GLfloat scale = GLfloat(i) / SSAOKernelSize;

		//Aceleramos la interpolación usando lerp, a mayor distancia, menos muestras.
		scale = 0.1f + scale * scale * (1.0f - 0.1f);
		sample *= scale;
		ssaoKernel.push_back(sample);
	}
}

void GenerateNoiseTexture()
{
	//Textura de ruido
	std::vector<glm::vec3> ssaoNoise;
	for (GLuint i = 0; i < SSAONoiseTextureSize*SSAONoiseTextureSize; i++)
	{
		//Rotación
		glm::vec3 noise(randomFloats(generator) * 2.0 - 1.0, randomFloats(generator) * 2.0 - 1.0, 0.0f);
		ssaoNoise.push_back(noise);
	}
	glGenTextures(1, &noiseTexture);
	glBindTexture(GL_TEXTURE_2D, noiseTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, SSAONoiseTextureSize, SSAONoiseTextureSize, 0, GL_RGB, GL_FLOAT, &ssaoNoise[0]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
}

void display()
{
	glm::vec3 lightPos = glm::vec3(0.0, 1.0, 0.0);
	glm::vec3 lightColor = glm::vec3(0.9, 0.1, 0.1);

	//Pasada de Geometría: renderizamos la geometria y el color de la escena en el gBuffer

	glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glm::mat4 projection = glm::perspective(45.0f, 1.0f * g_Width / g_Height, 1.0f, 100.0f);
	glm::vec3 cameraPos = vec3(6.0f * cos(yrot / 100), 2.0f * sin(xrot / 100) + 1.0f, 6.0f * sin(yrot / 100) * cos(xrot / 100));
	glm::mat4 view = glm::lookAt(cameraPos, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	glm::mat4 model;

	glm::mat4 mvp;
	glm::mat4 mv;
	glm::mat3 nm;

	glUseProgram(programID1);

	//Cubo que forma la habitación
	//Escalamos varios cubos ya que escalar un cubo para que sea la habitación haría que la OA no se viera bien, ya que
	//no funciona del todo bien con superficies totalmente planas
	model = glm::mat4();
	model = glm::translate(model, glm::vec3(-10.0f, 0.0f, -1.0f));
	model = glm::scale(model, glm::vec3(1.0f, 20.0f, 20.0f));

	mvp = projection * view * model;
	mv = view * model;
	nm = glm::mat3(glm::transpose(glm::inverse(mv)));
	glUniformMatrix4fv(locUniformMVP, 1, GL_FALSE, &mvp[0][0]);
	glUniformMatrix4fv(locUniformMV, 1, GL_FALSE, &mv[0][0]);
	glUniformMatrix3fv(locUniformNM, 1, GL_FALSE, &nm[0][0]);
	RenderCube();
	model = glm::mat4();
	model = glm::translate(model, glm::vec3(0.0f, 0.0f, 9.0f));
	model = glm::scale(model, glm::vec3(20.0f, 20.0f, 1.0f));

	mvp = projection * view * model;
	mv = view * model;
	nm = glm::mat3(glm::transpose(glm::inverse(mv)));
	glUniformMatrix4fv(locUniformMVP, 1, GL_FALSE, &mvp[0][0]);
	glUniformMatrix4fv(locUniformMV, 1, GL_FALSE, &mv[0][0]);
	glUniformMatrix3fv(locUniformNM, 1, GL_FALSE, &nm[0][0]);
	RenderCube();
	model = glm::mat4();
	model = glm::translate(model, glm::vec3(0.0f, 0.0f, -11.0f));
	model = glm::scale(model, glm::vec3(20.0f, 20.0f, 1.0f));

	mvp = projection * view * model;
	mv = view * model;
	nm = glm::mat3(glm::transpose(glm::inverse(mv)));
	glUniformMatrix4fv(locUniformMVP, 1, GL_FALSE, &mvp[0][0]);
	glUniformMatrix4fv(locUniformMV, 1, GL_FALSE, &mv[0][0]);
	glUniformMatrix3fv(locUniformNM, 1, GL_FALSE, &nm[0][0]);
	RenderCube();
	model = glm::mat4();
	model = glm::translate(model, glm::vec3(0.0f, 10.0f, -1.0f));
	model = glm::scale(model, glm::vec3(20.0f, 1.0f, 20.0f));

	mvp = projection * view * model;
	mv = view * model;
	nm = glm::mat3(glm::transpose(glm::inverse(mv)));
	glUniformMatrix4fv(locUniformMVP, 1, GL_FALSE, &mvp[0][0]);
	glUniformMatrix4fv(locUniformMV, 1, GL_FALSE, &mv[0][0]);
	glUniformMatrix3fv(locUniformNM, 1, GL_FALSE, &nm[0][0]);
	RenderCube();
	model = glm::mat4();
	model = glm::translate(model, glm::vec3(0.0, -1.5f, -1.0f));
	model = glm::scale(model, glm::vec3(20.0f, 1.0f, 20.0f));

	mvp = projection * view * model;
	mv = view * model;
	nm = glm::mat3(glm::transpose(glm::inverse(mv)));
	glUniformMatrix4fv(locUniformMVP, 1, GL_FALSE, &mvp[0][0]);
	glUniformMatrix4fv(locUniformMV, 1, GL_FALSE, &mv[0][0]);
	glUniformMatrix3fv(locUniformNM, 1, GL_FALSE, &nm[0][0]);
	RenderCube();

	// Dibuja Objetos
	model = glm::mat4();
	model = glm::rotate(model, (glm::mediump_float)-90, glm::vec3(1, 0, 0));
	model = glm::translate(model, glm::vec3(0.0, 0.0f, 0.0f));
	model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));

	mvp = projection * view * model;
	mv = view * model;
	nm = glm::mat3(glm::transpose(glm::inverse(mv)));
	glUniformMatrix4fv(locUniformMVP, 1, GL_FALSE, &mvp[0][0]);
	glUniformMatrix4fv(locUniformMV, 1, GL_FALSE, &mv[0][0]);
	glUniformMatrix3fv(locUniformNM, 1, GL_FALSE, &nm[0][0]);
	drawObj1();

	model = glm::mat4();
	model = glm::rotate(model, (glm::mediump_float) - 0, glm::vec3(1, 0, 0));
	model = glm::translate(model, glm::vec3(2.0, 0.0f, 0.0f));
	model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));

	mvp = projection * view * model;
	mv = view * model;
	nm = glm::mat3(glm::transpose(glm::inverse(mv)));
	glUniformMatrix4fv(locUniformMVP, 1, GL_FALSE, &mvp[0][0]);
	glUniformMatrix4fv(locUniformMV, 1, GL_FALSE, &mv[0][0]);
	glUniformMatrix3fv(locUniformNM, 1, GL_FALSE, &nm[0][0]);
	drawObj2();

	model = glm::mat4();
	model = glm::rotate(model, (glm::mediump_float) - 90, glm::vec3(1, 0, 0));
	model = glm::translate(model, glm::vec3(-2.0, 0.0f, -0.3f));
	model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));

	mvp = projection * view * model;
	mv = view * model;
	nm = glm::mat3(glm::transpose(glm::inverse(mv)));
	glUniformMatrix4fv(locUniformMVP, 1, GL_FALSE, &mvp[0][0]);
	glUniformMatrix4fv(locUniformMV, 1, GL_FALSE, &mv[0][0]);
	glUniformMatrix3fv(locUniformNM, 1, GL_FALSE, &nm[0][0]);
	drawObj3();

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	//Pasada SSAO
	glBindFramebuffer(GL_FRAMEBUFFER, ssaoFBO);
	glClear(GL_COLOR_BUFFER_BIT);
	glUseProgram(programID3);

	glUniform1f(locUniformSSAORadius, SSAORadius);
	glUniform1f(locUniformSSAOBias, SSAOBias);
	glUniform1f(locUniformSSAOPower, SSAOPower);
	glUniform1f(locUniformSSAOKernelSize, SSAOKernelSize);
	glUniform2f(locUniformSSAONoiseTexture, g_Width / (float)SSAONoiseTextureSize, g_Height / (float)SSAONoiseTextureSize);
	
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, gPositionTex);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, gNormalTex);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, noiseTexture);

	for (GLuint i = 0; i < SSAOKernelSize; ++i)
		glUniform3fv(locUniformSSAOKernel[i], 1, &ssaoKernel[i][0]);
	glUniformMatrix4fv(locUniformP, 1, GL_FALSE, glm::value_ptr(projection));
	RenderQuad();
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	
	//Pasada Blur: Usamos la textura Blur SSAO para eliminar ruido
	glBindFramebuffer(GL_FRAMEBUFFER, ssaoBlurFBO);
	glClear(GL_COLOR_BUFFER_BIT);
	glUseProgram(programID4);
	glUniform1i(locUniformDrawMode1, draw_mode);
	glUniform1i(locUniformSSAONoiseTextureSize, SSAONoiseTextureSize);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, ssaoColorBuffer);
	RenderQuad();
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	//Por último, pasada de los shaders de iluminación, en este caso el modelo Blinn-Phong con la SSAO incluida
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glUseProgram(programID2);

	//Texturas con la geometría de la escena
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, gPositionTex);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, gNormalTex);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, gAlbedoTex);

	//Textura con la SSAO (y el blur)
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, ssaoColorBufferBlur);
	glm::vec3 lightPosView = glm::vec3(view * glm::vec4(lightPos, 1.0));
	glUniform3fv(locUniformLightPosition, 1, &lightPosView[0]);
	glUniform3fv(locUniformLightColor, 1, &lightColor[0]);

	//Constantes de iluminación (que podrían cambiarse)
	//La variable constant es 1.0 en este caso, por eso no la enviamos al shader
	const GLfloat constant = 1.0;
	const GLfloat linear = 0.09;
	const GLfloat quadratic = 0.072;
	glUniform1f(locUniformLightLinear, linear);
	glUniform1f(locUniformLightQuadratic, quadratic);
	glUniform1i(locUniformDrawMode2, draw_mode);
	RenderQuad();

	glUseProgram(0);

	glutSwapBuffers();
}

void resize(int w, int h)
{
	g_Width = w;
	g_Height = h;
	glViewport(0, 0, g_Width, g_Height);
}

void idle()
{
	if (!mouseDown && animation)
	{
		xrot += 0.3f;
		yrot += 0.4f;
	}
	glutPostRedisplay();
}

void keyboard(unsigned char key, int x, int y)
{
	switch (key)
	{
	case 27: case 'q': case 'Q':
		exit(1);
		break;
	case 'a': case 'A':
		animation = !animation;
		break;
	case 'r': case 'R':
		property_mode = 0;
		std::cout << "Radio seleccionado" << std::endl;
		std::cout << "Valor actual: " << SSAORadius << std::endl;
		break;
	case 'b': case 'B':
		property_mode = 1;
		std::cout << "Bias seleccionado" << std::endl;
		std::cout << "Valor actual: " << SSAOBias << std::endl;
		break;
	case 'p': case 'P':
		property_mode = 2;
		std::cout << "Power seleccionado" << std::endl;
		std::cout << "Valor actual: " << SSAOPower << std::endl;
		break;
	case 'k': case 'K':
		property_mode = 3;
		std::cout << "Tamanyo del kernel seleccionado" << std::endl;
		std::cout << "Valor actual: " << SSAOKernelSize << std::endl;
		break;
	case 'n': case 'N':
		property_mode = 4;
		std::cout << "Textura de ruido seleccionada" << std::endl;
		std::cout << "Valor actual: " << SSAONoiseTextureSize << std::endl;
		break;
	case 'e': case 'E':
		property_mode = 5;
		ChangeProperty(true);
		std::cout << "Valores predeterminados establecidos" << std::endl;
		break;
	case '0':
		draw_mode = 0;
		break;
	case '1':
		draw_mode = 1;
		break;
	case '2':
		draw_mode = 2;
		break;
	case '3':
		draw_mode = 3;
		break;
	case '4':
		draw_mode = 4;
		break;
	case '5':
		draw_mode = 5;
		break;
	case '6':
		draw_mode = 6;
		break;
	case 'z': case 'Z': case '+':
		ChangeProperty(true);
		break;
	case 'x': case 'X': case'-':
		ChangeProperty(false);
		break;
	}
}

void ChangeProperty(bool increase)
{
	switch (property_mode)
	{
	case 0:
		ChangeSSOORadius(increase);
		break;
	case 1:
		ChangeSSOOBias(increase);
		break;
	case 2:
		ChangeSSOOPower(increase);
		break;
	case 3:
		ChangeSSOOKernelSize(increase);
		break;
	case 4:
		ChangeSSOONoiseTextureSize(increase);
		break;
	case 5:
		SSAORadius = 0.5f;
		SSAOBias = 0.025f;
		SSAOPower = 1.0f;
		SSAOKernelSize = 64;
		GenerateSampleKernel();
		SSAONoiseTextureSize = 4;
		GenerateNoiseTexture();
		break;
	}
}

void ChangeSSOORadius(bool increase)
{
	if (increase)
		SSAORadius += 0.5f;
	else
		SSAORadius -= 0.5f;

	if (SSAORadius > SSAORadiusMax)
		SSAORadius = SSAORadiusMax;
	else if (SSAORadius < SSAORadiusMin)
		SSAORadius = SSAORadiusMin;
	else
		std::cout << "Valor actual: " << SSAORadius << std::endl;
}

void ChangeSSOOBias(bool increase)
{
	if (increase)
		SSAOBias += 0.01f;
	else
		SSAOBias -= 0.01f;

	if (SSAOBias > SSAOBiasMax)
		SSAOBias = SSAOBiasMax;
	else if (SSAOBias < SSAOBiasMin)
		SSAOBias = SSAOBiasMin;
	else
		std::cout << "Valor actual: " << SSAOBias << std::endl;
}

void ChangeSSOOPower(bool increase)
{
	if (increase)
		SSAOPower += 1.00f;
	else
		SSAOPower -= 1.00f;

	if (SSAOPower > SSAOPowerMax)
		SSAOPower = SSAOPowerMax;
	else if (SSAOPower < SSAOPowerMin)
		SSAOPower = SSAOPowerMin;
	else
		std::cout << "Valor actual: " << SSAOPower << std::endl;
}

void ChangeSSOOKernelSize(bool increase)
{
	if (increase)
		SSAOKernelSize *= 2;
	else
		SSAOKernelSize /= 2;

	if (SSAOKernelSize > MAXKERNELSIZE)
		SSAOKernelSize = MAXKERNELSIZE;
	else if (SSAOKernelSize < SSAOKernelSizeMin)
		SSAOKernelSize = SSAOKernelSizeMin;
	else
	{
		GenerateSampleKernel();
		std::cout << "Valor actual: " << SSAOKernelSize << std::endl;
	}
}

void ChangeSSOONoiseTextureSize(bool increase)
{
	if (increase)
		SSAONoiseTextureSize *= 2;
	else
		SSAONoiseTextureSize /= 2;

	if (SSAONoiseTextureSize > SSAONoiseTextureSizeMax)
		SSAONoiseTextureSize = SSAONoiseTextureSizeMax;
	else if (SSAONoiseTextureSize < SSAONoiseTextureSizeMin)
		SSAONoiseTextureSize = SSAONoiseTextureSizeMin;
	else
	{
		GenerateNoiseTexture();
		std::cout << "Valor actual: " << SSAONoiseTextureSize << std::endl;
	}
}

void specialKeyboard(int key, int x, int y)
{
	if (key == GLUT_KEY_F1)
	{
		fullscreen = !fullscreen;

		if (fullscreen)
			glutFullScreen();
		else
		{
			glutReshapeWindow(g_Width, g_Height);
			glutPositionWindow(50, 50);
		}
	}
}

void mouse(int button, int state, int x, int y)
{
	if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN)
	{
		mouseDown = true;

		xdiff = x - yrot;
		ydiff = -y + xrot;
	}
	else
		mouseDown = false;
}

void mouseMotion(int x, int y)
{
	if (mouseDown)
	{
		yrot = x - xdiff;
		xrot = y + ydiff;

		glutPostRedisplay();
	}
}
