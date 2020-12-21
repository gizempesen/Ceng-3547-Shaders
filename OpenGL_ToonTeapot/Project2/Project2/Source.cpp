/*
* Program name: Assignment 1 (Real Time Rendering)
* Written by: Ben Thompson
* Date:12/10/2016
* 

SINCE GLFW DOESN'T SUPPORT WRITING TEXT TO SCREEN EXTRA LIBRARIES WERE REQUIRED
WHICH AT THIS POINT ARE NOT FULLY IMPLEMENTED... THE USER INPUT CONTROLS ARE AS 
FOLLOWS

CONTROLS:

USING NUMBER TO SWITCH BETWEEN SHADERS
1: VERTEX MANIPULATION WITH AMBIENT,DIFFUSE AND SPECULAR LIGHTING
2: TOON SHADER
3: BLINN-PHONG

ROTATING OBJECT:
P: (DO SUPERMAN THING) SPEEDS UP RIGHT ON REALEASE
O: (DO SUPERMAN THING) SPEEDS UP LEFT ON REALEASE

FIRST PERSON CAMERA MOVEMENT FROM HTTP://LEARNOPENGL.COM (SEE CAMERA INTERNAL HEADER FILE) 
W A D S AND MOUSE MOVEMENT

shader class: HTTP://LEARNOPENGL.COM 

*/

#include <iostream>
#include <cmath>

// GLEW
#define GLEW_STATIC
#include <GL/glew.h>

// GLFW
#include <GLFW/glfw3.h>

// Other Libraries
//Texture Library 
#include <SOIL.h>

// GLM Mathematics
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <string.h>

/*
* To implement Text printing (half implemented)
* */
// FreeType Headers
#include <ft2build.h>
#include <freetype/freetype.h>
#include <freetype/ftglyph.h>
#include <freetype/ftoutln.h>
#include <freetype/fttrigon.h>

// for text 
#include <GL/freeglut.h>

//ftgl
#include <FTGL/ftgl.h>

// Internal header files
#include "Shader.h"
#include "Camera.h"
#include "Teapot.h"



// Function prototypes
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void do_movement();
void set_rotation();
void choose_lighting();
void drawtext(const char *text, int length, int x, int y);//(has bug)

// Window dimensions
const GLuint WIDTH = 800, HEIGHT = 600;

// Camera
Camera  camera(glm::vec3(0.0f, 0.0f, 3.0f));
GLfloat lastX = WIDTH / 2.0;
GLfloat lastY = HEIGHT / 2.0;
bool    keys[1024];

// Light attributes
glm::vec3 lightPos(1.0f, 1.0f, 2.0f);

// Deltatime
GLfloat deltaTime = 0.0f;	// Time between current frame and last frame
GLfloat lastFrame = 0.0f;  	// Time of last frame

//rotation variable
GLfloat rotateSpeed = 45.0f;

//chosing light
GLint lightingChoice=2;

/*
*Shader Options
*/


//Specular Ambient Diffuse options(lighting1)
GLint specValue = 256;
GLfloat ambientStrength = 0.5f;
GLfloat specularStrength = 0.5f;
//Toon Shader (lighting2)


//Blinn Shader options (lighting3)
GLboolean blinn = true;


//string for displaying Shader name
std::string outputText;

// The MAIN function, from here we start the application and run the game loop
int main()
{
	
	// Init GLFW
	glfwInit();
	// Set all the required options for GLFW
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

	// Create a GLFWwindow object that we can use for GLFW's functions
	GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "TeapotOpenGL", nullptr, nullptr);
	glfwMakeContextCurrent(window);

	// Set the required callback functions
	glfwSetKeyCallback(window, key_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);

	// GLFW Options
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	// Set this to true so GLEW knows to use a modern approach to retrieving function pointers and extensions
	glewExperimental = GL_TRUE;
	// Initialize GLEW to setup the OpenGL Function pointers
	glewInit();

	// Define the viewport dimensions
	glViewport(0, 0, WIDTH, HEIGHT);

	// OpenGL options
	glEnable(GL_DEPTH_TEST);


	// Build and compile our shader programs
	Shader lighting1("shaders/lab1_1.vs", "shaders/lab1_1.frag");
	Shader lighting2("shaders/lighting2.vs", "shaders/lighting2.frag");
	Shader lighting3("shaders/lighting3.vs", "shaders/lighting3.frag");

	//learnopengl.com shader
	Shader lampShader("shaders/lamp.vs", "shaders/lamp.frag");


	/*
	* Defining teapot
	* */

	GLuint teapotVAO,teapot_vbo, normals_teapot_vbo;
	glGenBuffers(1, &teapot_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, teapot_vbo);


	// was to copy points from the header file into our VBO on graphics hardware
	glBufferData(GL_ARRAY_BUFFER, sizeof(teapot_vertex_points),
		teapot_vertex_points, GL_STATIC_DRAW);

	//Getting the normals
	glGenBuffers(1, &normals_teapot_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, normals_teapot_vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(teapot_normals),
		teapot_normals, GL_STATIC_DRAW);
	
	//vertex array VAO
	glGenVertexArrays(1, &teapotVAO);
	glBindVertexArray(teapotVAO);
	glEnableVertexAttribArray(0);
	
	//vbo array structure
	glBindBuffer(GL_ARRAY_BUFFER, teapot_vbo);
	glVertexAttribPointer(0,3, GL_FLOAT, GL_FALSE, 0,NULL);
	glEnableVertexAttribArray(1);
	
	//normals array structure
	glBindBuffer(GL_ARRAY_BUFFER, normals_teapot_vbo);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, NULL);

	/*
	* Defining box for lamp object
	*/

	GLfloat vertices[] = {
		-0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
		0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
		0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
		0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
		-0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
		-0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,

		-0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,
		0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,
		0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,
		0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,
		-0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,
		-0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,

		-0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
		-0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
		-0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
		-0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
		-0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
		-0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,

		0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
		0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
		0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
		0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
		0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
		0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,

		-0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
		0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
		0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
		0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
		-0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
		-0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,

		-0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
		0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
		0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
		0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
		-0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
		-0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f
	};

	
	// Lamps VAO and VBO
	GLuint VBO , lightVAO;
	glGenVertexArrays(1, &lightVAO);
	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	glBindVertexArray(lightVAO);

	// Position attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(0);

	// Normal attribute
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
	glEnableVertexAttribArray(1);
	glBindVertexArray(0);


	// Game loop
	while (!glfwWindowShouldClose(window))
	{
		// Calculate deltatime of current frame
		GLfloat currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		// Check if any events have been activiated (key pressed, mouse moved etc.) and call corresponding response functions
		glfwPollEvents();
		do_movement();
		set_rotation();
		choose_lighting();

		// Clear the colorbuffer
		glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);



		//attempt at using FTGL library (bug)
		//FTGLPixmapFont font("Users\BenTh\Desktop\Fonts\arial.ttf");
		//font.Render("Hello World");

		// Use cooresponding shader when setting uniforms/drawing objects
		GLint objectColorLoc;
		GLint lightColorLoc;
		GLint lightPosLoc;
		GLint viewPosLoc;

		glm::mat4 view;
		glm::mat4 projection;

		GLint modelLoc;
		GLint viewLoc;
		GLint projLoc;

		switch (lightingChoice)
		{
			case 1:
			{
				lighting1.Use();

				/*
				* Getting uniform matrix's in shader
				*/

				//local Object colour
				objectColorLoc = glGetUniformLocation(lighting1.Program, "objectColor");
				//local Light Color 
				lightColorLoc = glGetUniformLocation(lighting1.Program, "lightColor");
				//local light Position
				lightPosLoc = glGetUniformLocation(lighting1.Program, "lightPos");
				//local view Position
				viewPosLoc = glGetUniformLocation(lighting1.Program, "viewPos");
				

				/*
				* Setting uniform matrix's in shaders
				*/
				//setting light Position
				glUniform3f(lightPosLoc, lightPos.x, lightPos.y, lightPos.z);
				//setting object colour
				glUniform3f(objectColorLoc, 1.0f, 1.0f, 1.0f);
				//setting light colour
				glUniform3f(lightColorLoc, 1.0f, 1.0f, 1.0f);
				//setting view(cammera) position 
				glUniform3f(viewPosLoc, camera.Position.x, camera.Position.y, camera.Position.z);

				// Create camera transformations
				view = camera.GetViewMatrix();
				projection = glm::perspective(camera.Zoom, (GLfloat)WIDTH / (GLfloat)HEIGHT, 0.1f, 100.0f);
				// Get the uniform matrixes
				modelLoc = glGetUniformLocation(lighting1.Program, "model");
				viewLoc = glGetUniformLocation(lighting1.Program, "view");
				projLoc = glGetUniformLocation(lighting1.Program, "projection");
				
				// Pass the matrices for view and projection
				glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
				glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

				//setting option varibales for lighting 1
				glUniform1i(glGetUniformLocation(lighting1.Program, "SpecularValue"), specValue);
				glUniform1f(glGetUniformLocation(lighting1.Program, "ambientStrength"), ambientStrength);
				glUniform1f(glGetUniformLocation(lighting1.Program, "specularStrength"), specularStrength);

				break;
			}
			case 2:
			{
				lighting2.Use();
				
				/*
				* Getting uniform matrix's in shader
				*/

				//local Object colour
				objectColorLoc = glGetUniformLocation(lighting2.Program, "objectColor");
				//local Light Color 
				lightColorLoc = glGetUniformLocation(lighting2.Program, "lightColor");
				//local light Position
				lightPosLoc = glGetUniformLocation(lighting2.Program, "lightPos");
				//local view Position
				viewPosLoc = glGetUniformLocation(lighting2.Program, "viewPos");

				/*
				* Setting uniform matrix's in shaders
				*/
				//setting light Position
				glUniform3f(lightPosLoc, lightPos.x, lightPos.y, lightPos.z);
				//setting object colour
				glUniform3f(objectColorLoc, .7f, .7f, .7f);
				//setting light colour
				glUniform3f(lightColorLoc, 1.0f, 1.0f, 1.0f);
				//setting view(cammera) position 
				glUniform3f(viewPosLoc, camera.Position.x, camera.Position.y, camera.Position.z);

				// Create camera transformations
				view = camera.GetViewMatrix();
				projection = glm::perspective(camera.Zoom, (GLfloat)WIDTH / (GLfloat)HEIGHT, 0.1f, 100.0f);



				// Get the uniform locations
				modelLoc = glGetUniformLocation(lighting2.Program, "model");
				viewLoc = glGetUniformLocation(lighting2.Program, "view");
				projLoc = glGetUniformLocation(lighting2.Program, "projection");
				// Pass the matrices to the shader
				glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
				glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));
				break;
			}
			case 3:
			{

				// Draw objects
				lighting3.Use();


				//local Object colour
				objectColorLoc = glGetUniformLocation(lighting3.Program, "objectColor");
				//local Light Color 
				lightColorLoc = glGetUniformLocation(lighting3.Program, "lightColor");
				//local light Position
				lightPosLoc = glGetUniformLocation(lighting3.Program, "lightPos");
				//local view Position
				viewPosLoc = glGetUniformLocation(lighting3.Program, "viewPos");

				/*
				* Setting uniform matrix's in shaders
				*/
				//setting light Position
				glUniform3f(lightPosLoc, lightPos.x, lightPos.y, lightPos.z);
				//setting object colour
				glUniform3f(objectColorLoc, .8f, .8f, .8f);
				//setting light colour
				glUniform3f(lightColorLoc, 1.0f, 1.0f, 1.0f);
				//setting view(cammera) position 
				glUniform3f(viewPosLoc, camera.Position.x, camera.Position.y, camera.Position.z);



				// Create camera transformations
				view = camera.GetViewMatrix();
				projection = glm::perspective(camera.Zoom, (GLfloat)WIDTH / (GLfloat)HEIGHT, 0.1f, 100.0f);
				

				// Get the uniform locations
				modelLoc = glGetUniformLocation(lighting3.Program, "model");
				viewLoc = glGetUniformLocation(lighting3.Program, "view");
				projLoc = glGetUniformLocation(lighting3.Program, "projection");
				// Pass the matrices to the shader
				glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
				glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));
				
				
				/*
				* Light 
				*/
				//if blinn true or false
				glUniform1i(glGetUniformLocation(lighting3.Program, "blinn"), blinn);
			

				break;
			}
		}


		//draw teapot
		glBindVertexArray(teapotVAO);
		glm::mat4 model;
		model = glm::rotate(model, (GLfloat)glfwGetTime() * glm::radians(rotateSpeed), glm::vec3(0.0, 1.0, 0.0));
		model = glm::scale(model, glm::vec3(0.1f));
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		glDrawArrays(GL_TRIANGLES, 0, teapot_vertex_count);
		glBindVertexArray(0);
		
		
		//Draw the lamp object, again binding the appropriate shader
		lampShader.Use();
		// Get location objects for the matrices on the lamp shader (these could be different on a different shader)
		modelLoc = glGetUniformLocation(lampShader.Program, "model");
		viewLoc = glGetUniformLocation(lampShader.Program, "view");
		projLoc = glGetUniformLocation(lampShader.Program, "projection");
		// Set matrices
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));
		model = glm::mat4();
		model = glm::translate(model, lightPos);
		model = glm::scale(model, glm::vec3(0.2f)); // Make it a smaller cube
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		
		// Draw the light object (using light's vertex attributes)
		glBindVertexArray(lightVAO);
		glDrawArrays(GL_TRIANGLES, 0, 36);
		glBindVertexArray(0);



		//draw text (bug)
		/* 
		glColor3f(.1f, .1f, .1f);
	    drawtext(outputText.data(), outputText.size(), 0, 0);
		*/

		// Swap the screen buffers
		glfwSwapBuffers(window);
	}

	// Terminate GLFW, clearing any resources allocated by GLFW.
	glfwTerminate();
	return 0;
}

// Is called whenever a key is pressed/released via GLFW
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);
	if (key >= 0 && key < 1024)
	{
		if (action == GLFW_PRESS)
			keys[key] = true;
		else if (action == GLFW_RELEASE)
			keys[key] = false;
	}
}

void do_movement()
{
	// Camera controls
	if (keys[GLFW_KEY_W])
		camera.ProcessKeyboard(FORWARD, deltaTime);
	if (keys[GLFW_KEY_S])
		camera.ProcessKeyboard(BACKWARD, deltaTime);
	if (keys[GLFW_KEY_A])
		camera.ProcessKeyboard(LEFT, deltaTime);
	if (keys[GLFW_KEY_D])
		camera.ProcessKeyboard(RIGHT, deltaTime);
}


void set_rotation() {
	if (keys[GLFW_KEY_P])
		rotateSpeed += 0.1f;
	if (keys[GLFW_KEY_O])
		rotateSpeed -= 0.1f;
}

void choose_lighting() {

	if (keys[GLFW_KEY_1])
		lightingChoice = 1;
	if (keys[GLFW_KEY_2])
		lightingChoice = 2;
	if (keys[GLFW_KEY_3])
		lightingChoice = 3;
}

bool firstMouse = true;
void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	if (firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	GLfloat xoffset = xpos - lastX;
	GLfloat yoffset = lastY - ypos;  // Reversed since y-coordinates go from bottom to left

	lastX = xpos;
	lastY = ypos;

	camera.ProcessMouseMovement(xoffset, yoffset);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	camera.ProcessMouseScroll(yoffset);
}

void drawtext(const char *text, int length, int x, int y)
{
	glMatrixMode(GL_PROJECTION);
	double *matrix = new double[16];
	glGetDoublev(GL_PROJECTION_MATRIX, matrix);
	glLoadIdentity();
	glOrtho(0, HEIGHT, 0, WIDTH, -5, 5);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glPushMatrix();
	glLoadIdentity();
	glRasterPos2i(x,y);
	for (int i = 0; i < length; i++)
	{
		glutBitmapCharacter(GLUT_BITMAP_9_BY_15, (int)text[i]);

	}
	glPopMatrix();
	glMatrixMode(GL_PROJECTION);
	glLoadMatrixd(matrix);
	glMatrixMode(GL_MODELVIEW);

}