#include <iostream>         // cout, cerr
#include <cstdlib>          // EXIT_FAILURE
#include <GL/glew.h>        // GLEW library
#include <GLFW/glfw3.h>     // GLFW library

// GLM Math Header inclusions
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"      // Image loading Utility functions

#include "meshes.h"

#include "camera.h"

// Uses the standard namespace for debug output
using namespace std;

// Unnamed namespace for C++ defines //
namespace
{
	// Macro for OpenGL window title
	const char* const WINDOW_TITLE = "Ashlyn Godwin FINAL PROJECT";

	// Variables for window width and height
	const int WINDOW_WIDTH = 1600;
	const int WINDOW_HEIGHT = 900;

	// Main GLFW window
	GLFWwindow* gWindow = nullptr;
	// Shader program
	GLuint gProgramId1;
	GLuint gProgramId2;
	// Texture Ids
	GLuint gTextureId; //brick (unused)
	GLuint gTextureId2; // aluminum
	GLuint gTextureId3; //black stripes (cap)
	GLuint gTextureId4; //red + white pattern for trimmer line
	GLuint gTextureId5; //label for trimmer spool
	GLuint gTextureId6; //gas can label
	GLuint gTextureId7; //chainsaw
	GLuint gTextureId8; //concrete
	GLuint gTextureId9; //red for trimmer box (substitute for untextured)

	Meshes meshes;

	Camera gCamera(glm::vec3(-20.0f, 50.0f, 50.0f));
	GLint gCurrentCameraIndex = 1;

	float gDeltaTime = 0.0f; // Time between current frame and last frame
	float gLastFrame = 0.0f;

	float gLastX = WINDOW_WIDTH / 2.0f;
	float gLastY = WINDOW_HEIGHT / 2.0f;
	bool gFirstMouse = true;
}

// User-defined Function prototypes //
bool Initialize(int, char* [], GLFWwindow** window);
void ProcessInput(GLFWwindow* window);
void Render();
bool CreateShaderProgram(const char* vtxShaderSource, const char* fragShaderSource, GLuint& programId);
void DestroyShaderProgram(GLuint programId);
bool CreateTexture(const char* filename, GLuint& textureId);
void DestroyTexture(GLuint textureId);
void UMousePositionCallback(GLFWwindow* window, double xpos, double ypos);
void UMouseScrollCallback(GLFWwindow* window, double xoffset, double yoffset);
void UMouseButtonCallback(GLFWwindow* window, int button, int action, int mods);

// Shader program Macro //
#ifndef GLSL
#define GLSL(Version, Source) "#version " #Version " core \n" #Source
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////////
/* Vertex Shader Source Code*/
const GLchar* vertexShaderSource1 = GLSL(440,

	layout(location = 0) in vec3 vertexPosition; // VAP position 0 for vertex position data
layout(location = 1) in vec3 vertexNormal; // VAP position 1 for normals
layout(location = 2) in vec2 textureCoordinate;

out vec3 vertexFragmentNormal; // For outgoing normals to fragment shader
out vec3 vertexFragmentPos; // For outgoing color / pixels to fragment shader
out vec2 vertexTextureCoordinate;

//Uniform / Global variables for the  transform matrices
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
	gl_Position = projection * view * model * vec4(vertexPosition, 1.0f); // Transforms vertices into clip coordinates

	vertexFragmentPos = vec3(model * vec4(vertexPosition, 1.0f)); // Gets fragment / pixel position in world space only (exclude view and projection)

	vertexFragmentNormal = mat3(transpose(inverse(model))) * vertexNormal; // get normal vectors in world space only and exclude normal translation properties
	vertexTextureCoordinate = textureCoordinate;
}
);
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
/* Fragment Shader Source Code*/
const GLchar* fragmentShaderSource1 = GLSL(440,

	in vec3 vertexFragmentNormal; // For incoming normals
in vec3 vertexFragmentPos; // For incoming fragment position
in vec2 vertexTextureCoordinate;

out vec4 fragmentColor; // For outgoing cube color to the GPU

// Uniform / Global variables for object color, light color, light position, and camera/view position
uniform vec4 objectColor;
uniform vec3 ambientColor;
uniform vec3 light1Color;
uniform vec3 light1Position;
uniform vec3 light2Color;
uniform vec3 light2Position;
uniform vec3 viewPosition;
uniform sampler2D uTexture; // Useful when working with multiple textures
uniform vec2 uvScale;
uniform bool ubHasTexture;
uniform float ambientStrength; // Set ambient or global lighting strength
uniform float specularIntensity1;
uniform float highlightSize1;
uniform float specularIntensity2;
uniform float highlightSize2;

void main()
{
	/*Phong lighting model calculations to generate ambient, diffuse, and specular components*/

	//Calculate Ambient lighting
	vec3 ambient = ambientStrength * ambientColor; // Generate ambient light color

	//**Calculate Diffuse lighting**
	vec3 norm = normalize(vertexFragmentNormal); // Normalize vectors to 1 unit
	vec3 light1Direction = normalize(light1Position - vertexFragmentPos); // Calculate distance (light direction) between light source and fragments/pixels on cube
	float impact1 = max(dot(norm, light1Direction), 0.0);// Calculate diffuse impact by generating dot product of normal and light
	vec3 diffuse1 = impact1 * light1Color; // Generate diffuse light color
	vec3 light2Direction = normalize(light2Position - vertexFragmentPos); // Calculate distance (light direction) between light source and fragments/pixels on cube
	float impact2 = max(dot(norm, light2Direction), 0.0);// Calculate diffuse impact by generating dot product of normal and light
	vec3 diffuse2 = impact2 * light2Color; // Generate diffuse light color

	//**Calculate Specular lighting**
	vec3 viewDir = normalize(viewPosition - vertexFragmentPos); // Calculate view direction
	vec3 reflectDir1 = reflect(-light1Direction, norm);// Calculate reflection vector
	//Calculate specular component
	float specularComponent1 = pow(max(dot(viewDir, reflectDir1), 0.0), highlightSize1);
	vec3 specular1 = specularIntensity1 * specularComponent1 * light1Color;
	vec3 reflectDir2 = reflect(-light2Direction, norm);// Calculate reflection vector
	//Calculate specular component
	float specularComponent2 = pow(max(dot(viewDir, reflectDir2), 0.0), highlightSize2);
	vec3 specular2 = specularIntensity2 * specularComponent2 * light2Color;

	//**Calculate phong result**
	//Texture holds the color to be used for all three components
	vec4 textureColor = texture(uTexture, vertexTextureCoordinate * uvScale);
	vec3 phong1;
	vec3 phong2;

	if (ubHasTexture == true)
	{
		phong1 = (ambient + diffuse1 + specular1) * textureColor.xyz;
		phong2 = (ambient + diffuse2 + specular2) * textureColor.xyz;
	}
	else
	{
		phong1 = (ambient + diffuse1 + specular1) * objectColor.xyz;
		phong2 = (ambient + diffuse2 + specular2) * objectColor.xyz;
	}

	fragmentColor = vec4(phong1 + phong2, 1.0); // Send lighting results to GPU
	//fragmentColor = vec4(1.0f, 1.0f, 1.0f, 1.0f);
}
);
/////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////

// main function. Entry point to the OpenGL program //
int main(int argc, char* argv[])
{
	if (!Initialize(argc, argv, &gWindow))
		return EXIT_FAILURE;

	// Create the mesh, send data to VBO
	meshes.CreateMeshes();

	// Create the shader program
	if (!CreateShaderProgram(vertexShaderSource1, fragmentShaderSource1, gProgramId1))
		return EXIT_FAILURE;

	// Load texture data from file
	//const char * texFilename1 = "../../resources/textures/blue_granite.jpg";
	//if (!CreateTexture(texFilename1, gTextureIdBlue))
	//{
	//	cout << "Failed to load texture " << texFilename1 << endl;
	//	return EXIT_FAILURE;
	//}
	//Texture Prep
	const char* texFilename = "brick.jpg";
	if (!CreateTexture(texFilename, gTextureId))
	{
		cout << "Failed to load texture " << texFilename << endl;
		return EXIT_FAILURE;
	}
	const char* texFilename2 = "alum.jpg";
	if (!CreateTexture(texFilename2, gTextureId2))
	{
		cout << "Failed to load texture " << texFilename2 << endl;
		return EXIT_FAILURE;
	}
	const char* texFilename3 = "capTex.jpg";
	if (!CreateTexture(texFilename3, gTextureId3))
	{
		cout << "Failed to load texture " << texFilename3 << endl;
		return EXIT_FAILURE;
	}
	const char* texFilename4 = "red+white.jpg";
	if (!CreateTexture(texFilename4, gTextureId4))
	{
		cout << "Failed to load texture " << texFilename4 << endl;
		return EXIT_FAILURE;
	}
	const char* texFilename5 = "spool_top.jpg";
	if (!CreateTexture(texFilename5, gTextureId5))
	{
		cout << "Failed to load texture " << texFilename5 << endl;
		return EXIT_FAILURE;
	}
	const char* texFilename6 = "gasCan.jpg";
	if (!CreateTexture(texFilename6, gTextureId6))
	{
		cout << "Failed to load texture " << texFilename6 << endl;
		return EXIT_FAILURE;
	}
	const char* texFilename7 = "chainsaw1.jpg";
	if (!CreateTexture(texFilename7, gTextureId7))
	{
		cout << "Failed to load texture " << texFilename7 << endl;
		return EXIT_FAILURE;
	}
	const char* texFilename8 = "concrete.jpg";
	if (!CreateTexture(texFilename8, gTextureId8))
	{
		cout << "Failed to load texture " << texFilename8 << endl;
		return EXIT_FAILURE;
	}
	const char* texFilename9 = "red.jpg";
	if (!CreateTexture(texFilename9, gTextureId9))
	{
		cout << "Failed to load texture " << texFilename9 << endl;
		return EXIT_FAILURE;
	}
	// Activate the program that will reference the texture
	glUseProgram(gProgramId1);
	// We set the texture as texture unit 0
	glUniform1i(glGetUniformLocation(gProgramId1, "uTexture"), 0);

	// Sets the background color of the window to black (it will be implicitely used by glClear)
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);


	// Render loop
	while (!glfwWindowShouldClose(gWindow))
	{
		float currentFrame = glfwGetTime();
		gDeltaTime = currentFrame - gLastFrame;
		gLastFrame = currentFrame;
		// Process keyboard input before rendering
		ProcessInput(gWindow);

		// Render this frame
		Render();

		glfwPollEvents();
	}

	// Release mesh data
	meshes.DestroyMeshes();
	// Release shader program
	DestroyShaderProgram(gProgramId1);
	// Release the textures
	//DestroyTexture(gTextureId);

	exit(EXIT_SUCCESS); // Terminates the program successfully
}

// Initialize GLFW, GLEW, and create a window //
bool Initialize(int argc, char* argv[], GLFWwindow** window)
{
	// GLFW: initialize and configure
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 4);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// GLFW: create OpenGL output window, return error if fails
	*window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_TITLE, NULL, NULL);
	if (*window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return false;
	}

	// Set the context for the current window
	glfwMakeContextCurrent(*window);
	glfwSetMouseButtonCallback(*window, UMouseButtonCallback);
	glfwSetScrollCallback(*window, UMouseScrollCallback);
	glfwSetCursorPosCallback(*window, UMousePositionCallback);
	// GLEW: initialize
	// ----------------
	// Note: if using GLEW version 1.13 or earlier
	glewExperimental = GL_TRUE;
	GLenum GlewInitResult = glewInit();
	// If init fails, output error string, return error
	if (GLEW_OK != GlewInitResult)
	{
		std::cerr << glewGetErrorString(GlewInitResult) << std::endl;
		return false;
	}

	// Displays GPU OpenGL version
	cout << "INFO: OpenGL Version: " << glGetString(GL_VERSION) << endl;

	return true;
}

// Process keyboard input
void ProcessInput(GLFWwindow* window)
{
	if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS)
	{
		if (gCurrentCameraIndex == 1)
		{
			gCurrentCameraIndex = 2;
		}
		else if (gCurrentCameraIndex == 2)
		{
			gCurrentCameraIndex = 1;
		}
	}
	static const float cameraSpeed = 2.5f;

	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		gCamera.ProcessKeyboard(FORWARD, gDeltaTime);
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		gCamera.ProcessKeyboard(BACKWARD, gDeltaTime);
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		gCamera.ProcessKeyboard(LEFT, gDeltaTime);
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		gCamera.ProcessKeyboard(RIGHT, gDeltaTime);
	if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
		gCamera.ProcessKeyboard(UP, gDeltaTime);
	if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
		gCamera.ProcessKeyboard(DOWN, gDeltaTime);
}

// Render the next frame to the OpenGL viewport //
void Render()
{
	/*     Uniform Declarations      */
	//Uniforms for Vertex Shader 
	GLint modelLoc;
	GLint viewLoc;
	GLint projectionLoc;


	//Uniform Locations for fragment shader
	GLint objectColorLoc;
	GLint ambientColorLoc;
	GLint light1ColorLoc;
	GLint light1PositionLoc;
	GLint light2ColorLoc;
	GLint light2PositionLoc;
	GLint viewPositionLoc;
	GLint uTextureLoc; // Useful when working with multiple textures
	GLint uvScaleLoc;
	GLint ubHasTextureLoc;
	GLint ambientStrengthLoc; // Set ambient or global lighting strength
	GLint specularIntensity1Loc;
	GLint highlightSize1Loc;
	GLint specularIntensity2Loc;
	GLint highlightSize2Loc;


	glm::mat4 projection;
	glm::mat4 model;
	glm::mat4 scale;
	glm::mat4 rotation;
	glm::mat4 translation;

	bool ubHasTextureVal;

	/*     Defining Uniform Locations     */
	// Retrieves and passes transform matrices to the Shader program. Utilized by individual objects in later sections
	modelLoc = glGetUniformLocation(gProgramId1, "model");
	viewLoc = glGetUniformLocation(gProgramId1, "view");
	projectionLoc = glGetUniformLocation(gProgramId1, "projection");
	objectColorLoc = glGetUniformLocation(gProgramId1, "objectColor");
	light1ColorLoc = glGetUniformLocation(gProgramId1, "light1Color");
	light1PositionLoc = glGetUniformLocation(gProgramId1, "light1Position");
	light2ColorLoc = glGetUniformLocation(gProgramId1, "light2Color");
	light2PositionLoc = glGetUniformLocation(gProgramId1, "light2Position");
	viewPositionLoc = glGetUniformLocation(gProgramId1, "viewPosition");
	uTextureLoc = glGetUniformLocation(gProgramId1, "uTexture");
	ubHasTextureLoc = glGetUniformLocation(gProgramId1, "ubHasTexture");
	ambientStrengthLoc = glGetUniformLocation(gProgramId1, "ambientStrength");
	specularIntensity1Loc = glGetUniformLocation(gProgramId1, "specularIntensity1");
	highlightSize1Loc = glGetUniformLocation(gProgramId1, "highlightSize1");
	specularIntensity2Loc = glGetUniformLocation(gProgramId1, "specularIntensity2");
	highlightSize2Loc = glGetUniformLocation(gProgramId1, "highlightSize2");
	uvScaleLoc = glGetUniformLocation(gProgramId1, "uvScale");
	ambientColorLoc = glGetUniformLocation(gProgramId1, "ambientColor");

	// Enable z-depth
	glEnable(GL_DEPTH_TEST);

	// Clear the background
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// allows camera to switch between perspective and orthographic projections
	switch (gCurrentCameraIndex)
	{
	case 1:
		projection = glm::perspective(glm::radians(50.0f), (GLfloat)WINDOW_WIDTH / (GLfloat)WINDOW_HEIGHT, 0.1f, 200.0f);
		break;
	case 2:
		projection = glm::ortho(-50.0f, 50.0f, -50.0f, 50.0f, 0.1f, 100.0f);
		break;
	}


	glm::mat4 view = gCamera.GetViewMatrix();
	// Set Active Shader Program
	glUseProgram(gProgramId1);


	//Set Universal Things (Will not change from object to object)
	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view)); // sends view data to projection loc which is then read by shader
	glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection)); // sends projection data to projection loc which is then read by shader
	glUniform3f(light1ColorLoc, 1.0f, 1.0f, 1.0f);
	glUniform3f(light1PositionLoc, 50.0f, 70.0f, 10.0f);
	glUniform3f(light2ColorLoc, 1.0f, 1.0f, 1.0f);
	glUniform3f(light2PositionLoc, -20.0f, 70.0f, 10.0f);
	const glm::vec3 cameraPosition = gCamera.Position;
	glUniform3f(viewPositionLoc, cameraPosition.x, cameraPosition.y, cameraPosition.z);
	glUniform3f(ambientColorLoc, 1.0, 1.0, 1.0);
	glUniform1f(ambientStrengthLoc, 0.1f);
	glUniform2f(uvScaleLoc, 1.0f, 1.0f);

	/*          TruFuel Can          */
	/*     Main Cylinder Body     */

	// Activate the VBOs contained within the mesh's VAO
	glBindVertexArray(meshes.gCylinderMesh.vao);

	// Set the mesh transfomation values
	scale = glm::scale(glm::vec3(3.0f, 8.0f, 3.0f));
	rotation = glm::rotate(0.0f, glm::vec3(1.0f, 1.0f, 1.0f));
	translation = glm::translate(glm::vec3(0.0f, 0.0f, 0.0f));
	model = translation * rotation * scale;


	ubHasTextureVal = true;
	glUniform1i(ubHasTextureLoc, ubHasTextureVal);


	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, gTextureId6);


	// Remaining Object Specific Uniforms
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	glUniformMatrix4fv(objectColorLoc, 1, GL_FALSE, glm::value_ptr(model));
	glUniform1f(specularIntensity1Loc, 1.0f);
	glUniform1f(specularIntensity2Loc, 1.0f);
	glUniform1f(highlightSize1Loc, 16.0f);
	glUniform1f(highlightSize2Loc, 16.0f);

	//glDrawArrays(GL_TRIANGLE_FAN, 0, 36);		//bottom
	//glDrawArrays(GL_TRIANGLE_FAN, 36, 72);		//top
	glDrawArrays(GL_TRIANGLE_STRIP, 72, 146);	//sides

	// Deactivate the VAO for box
	glBindVertexArray(0);

	/*     Tapered Aluminum Portion     */

	// Activate the VBOs contained within the mesh's VAO
	glBindVertexArray(meshes.gConeMesh.vao);

	// Set the mesh transfomation values
	scale = glm::scale(glm::vec3(3.0f, 2.0f, 3.0f));
	rotation = glm::rotate(0.0f, glm::vec3(1.0f, 1.0f, 1.0f));
	translation = glm::translate(glm::vec3(0.0f, 8.0f, 0.0f));
	model = translation * rotation * scale;


	ubHasTextureVal = true;
	glUniform1i(ubHasTextureLoc, ubHasTextureVal);


	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, gTextureId2);


	// Remaining Object Specific Uniforms
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	glUniformMatrix4fv(objectColorLoc, 1, GL_FALSE, glm::value_ptr(model));
	glUniform1f(specularIntensity1Loc, 1.0f);
	glUniform1f(specularIntensity2Loc, 1.0f);
	glUniform1f(highlightSize1Loc, 30.0f);
	glUniform1f(highlightSize2Loc, 30.0f);

	//glDrawElements(GL_TRIANGLES, meshes.gPyramid4Mesh.nIndices, GL_UNSIGNED_INT, (void*)0);
	glDrawArrays(GL_TRIANGLE_STRIP, 36, 108);

	// Deactivate the VAO for box
	glBindVertexArray(0);
	/*     Rim Around Aluminum     */
	// Activate the VBOs contained within the mesh's VAO
	glBindVertexArray(meshes.gTorusMesh.vao);

	// Set the mesh transfomation values
	scale = glm::scale(glm::vec3(2.9f, 2.9f, 1.0f));
	rotation = glm::rotate(1.57f, glm::vec3(1.0f, 0.0f, 0.0f));
	translation = glm::translate(glm::vec3(0.0f, 8.0f, 0.0f));
	model = translation * rotation * scale;


	ubHasTextureVal = true;
	glUniform1i(ubHasTextureLoc, ubHasTextureVal);


	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, gTextureId2);


	// Remaining Object Specific Uniforms
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	glUniformMatrix4fv(objectColorLoc, 1, GL_FALSE, glm::value_ptr(model));
	glUniform1f(specularIntensity1Loc, 1.0f);
	glUniform1f(specularIntensity2Loc, 1.0f);
	glUniform1f(highlightSize1Loc, 16.0f);
	glUniform1f(highlightSize2Loc, 16.0f);

	//glDrawElements(GL_TRIANGLES, meshes.gPyramid4Mesh.nIndices, GL_UNSIGNED_INT, (void*)0);
	glDrawArrays(GL_TRIANGLES, 0, meshes.gTorusMesh.nVertices);

	// Deactivate the VAO for box
	glBindVertexArray(0);
	/*     Cap     */
	// Activate the VBOs contained within the mesh's VAO
	glBindVertexArray(meshes.gCylinderMesh.vao);

	// Set the mesh transfomation values
	scale = glm::scale(glm::vec3(1.0f, 1.5f, 1.0f));
	rotation = glm::rotate(0.0f, glm::vec3(1.0f, 1.0f, 1.0f));
	translation = glm::translate(glm::vec3(0.0f, 9.0f, 0.0f));
	model = translation * rotation * scale;


	ubHasTextureVal = true;
	glUniform1i(ubHasTextureLoc, ubHasTextureVal);


	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, gTextureId3);


	// Remaining Object Specific Uniforms
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	glUniformMatrix4fv(objectColorLoc, 1, GL_FALSE, glm::value_ptr(model));
	glUniform1f(specularIntensity1Loc, 1.0f);
	glUniform1f(specularIntensity2Loc, 0.1f);
	glUniform1f(highlightSize1Loc, 16.0f);
	glUniform1f(highlightSize2Loc, 16.0f);

	glDrawArrays(GL_TRIANGLE_FAN, 0, 36);		//bottom
	glDrawArrays(GL_TRIANGLE_FAN, 36, 72);		//top
	glDrawArrays(GL_TRIANGLE_STRIP, 72, 146);	//sides

	// Deactivate the VAO for box
	glBindVertexArray(0);
	/*          Trimmer Spool          */

	/*     Torus     */

	// Activate the VBOs contained within the mesh's VAO
	glBindVertexArray(meshes.gTorusMesh.vao);

	// Set the mesh transfomation values
	scale = glm::scale(glm::vec3(8.0f, 8.0f, 12.0f));
	rotation = glm::rotate(1.57f, glm::vec3(1.0f, 0.0f, 0.0f));
	translation = glm::translate(glm::vec3(15.0f, 1.2f, 0.0f));
	model = translation * rotation * scale;


	ubHasTextureVal = true;
	glUniform1i(ubHasTextureLoc, ubHasTextureVal);


	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, gTextureId4);


	// Remaining Object Specific Uniforms
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	glUniformMatrix4fv(objectColorLoc, 1, GL_FALSE, glm::value_ptr(model));
	glUniform1f(specularIntensity1Loc, 0.1f);
	glUniform1f(specularIntensity2Loc, 0.1f);
	glUniform1f(highlightSize1Loc, 16.0f);
	glUniform1f(highlightSize2Loc, 16.0f);

	//glDrawElements(GL_TRIANGLES, meshes.gPyramid4Mesh.nIndices, GL_UNSIGNED_INT, (void*)0);
	glDrawArrays(GL_TRIANGLES, 0, meshes.gTorusMesh.nVertices);

	// Deactivate the VAO for box
	glBindVertexArray(0);
	// Flips the the back buffer with the front buffer every frame (refresh)

	/*     Inner Portion     */
	// Activate the VBOs contained within the mesh's VAO
	glBindVertexArray(meshes.gCylinderMesh.vao);

	// Set the mesh transfomation values
	scale = glm::scale(glm::vec3(8.0f, 2.4f, 8.0f));
	rotation = glm::rotate(0.0f, glm::vec3(1.0f, 0.0f, 0.0f));
	translation = glm::translate(glm::vec3(15.0f, 0.0f, 0.0f));
	model = translation * rotation * scale;


	ubHasTextureVal = true;
	glUniform1i(ubHasTextureLoc, ubHasTextureVal);


	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, gTextureId5);


	// Remaining Object Specific Uniforms
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	glUniformMatrix4fv(objectColorLoc, 1, GL_FALSE, glm::value_ptr(model));
	glUniform1f(specularIntensity1Loc, 0.1f);
	glUniform1f(specularIntensity2Loc, 0.1f);
	glUniform1f(highlightSize1Loc, .01f);
	glUniform1f(highlightSize2Loc, .01f);

	//glDrawElements(GL_TRIANGLES, meshes.gPyramid4Mesh.nIndices, GL_UNSIGNED_INT, (void*)0);
	//glDrawArrays(GL_TRIANGLE_FAN, 0, 36);		//bottom
	glDrawArrays(GL_TRIANGLE_FAN, 36, 72);		//top
	//glDrawArrays(GL_TRIANGLE_STRIP, 72, 146);	//sides

	// Deactivate the VAO for box
	glBindVertexArray(0);
	/*          Chainsaw Box          */
	/*     Box     */
	// Activate the VBOs contained within the mesh's VAO
	glBindVertexArray(meshes.gBoxMesh.vao);

	// Set the mesh transfomation values
	scale = glm::scale(glm::vec3(15.0f, 30.0f, 15.0f));
	rotation = glm::rotate(0.25f, glm::vec3(0.0f, 1.0f, 0.0f));
	translation = glm::translate(glm::vec3(-15.0f, 15.0f, 0.0f));
	model = translation * rotation * scale;


	ubHasTextureVal = true;
	glUniform1i(ubHasTextureLoc, ubHasTextureVal);


	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, gTextureId7);


	// Remaining Object Specific Uniforms
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	glUniformMatrix4fv(objectColorLoc, 1, GL_FALSE, glm::value_ptr(model));
	glUniform1f(specularIntensity1Loc, 0.1f);
	glUniform1f(specularIntensity2Loc, 0.1f);
	glUniform1f(highlightSize1Loc, 16.0f);
	glUniform1f(highlightSize2Loc, 16.0f);

	//glDrawElements(GL_TRIANGLES, meshes.gPyramid4Mesh.nIndices, GL_UNSIGNED_INT, (void*)0);
	glDrawElements(GL_TRIANGLES, meshes.gBoxMesh.nIndices, GL_UNSIGNED_INT, (void*)0);

	// Deactivate the VAO for box
	glBindVertexArray(0);
	/*          Trimmer Box          */
	/*     Big Box     */
	// Activate the VBOs contained within the mesh's VAO
	glBindVertexArray(meshes.gBoxMesh.vao);

	// Set the mesh transfomation values
	scale = glm::scale(glm::vec3(20.0f, 40.0f, 10.0f));
	rotation = glm::rotate(0.0f, glm::vec3(0.0f, 1.0f, 0.0f));
	translation = glm::translate(glm::vec3(-50.0f, 20.0f, 0.0f));
	model = translation * rotation * scale;


	ubHasTextureVal = true;
	glUniform1i(ubHasTextureLoc, ubHasTextureVal);


	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, gTextureId9);


	// Remaining Object Specific Uniforms
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	glUniformMatrix4fv(objectColorLoc, 1, GL_FALSE, glm::value_ptr(model));
	glUniform1f(specularIntensity1Loc, 1.0f);
	glUniform1f(specularIntensity2Loc, 0.1f);
	glUniform1f(highlightSize1Loc, 16.0f);
	glUniform1f(highlightSize2Loc, 16.0f);

	//glDrawElements(GL_TRIANGLES, meshes.gPyramid4Mesh.nIndices, GL_UNSIGNED_INT, (void*)0);
	glDrawElements(GL_TRIANGLES, meshes.gBoxMesh.nIndices, GL_UNSIGNED_INT, (void*)0);

	// Deactivate the VAO for box
	glBindVertexArray(0);
	/*     Small Box     */
	// Activate the VBOs contained within the mesh's VAO
	glBindVertexArray(meshes.gBoxMesh.vao);

	// Set the mesh transfomation values
	scale = glm::scale(glm::vec3(20.0f, 15.0f, 10.0f));
	rotation = glm::rotate(0.0f, glm::vec3(0.0f, 1.0f, 0.0f));
	translation = glm::translate(glm::vec3(-50.0f, 7.5f, 10.0f));
	model = translation * rotation * scale;


	ubHasTextureVal = true;
	glUniform1i(ubHasTextureLoc, ubHasTextureVal);


	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, gTextureId9);


	// Remaining Object Specific Uniforms
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	glUniformMatrix4fv(objectColorLoc, 1, GL_FALSE, glm::value_ptr(model));
	glUniform1f(specularIntensity1Loc, 1.0f);
	glUniform1f(specularIntensity2Loc, 0.1f);
	glUniform1f(highlightSize1Loc, 16.0f);
	glUniform1f(highlightSize2Loc, 16.0f);

	//glDrawElements(GL_TRIANGLES, meshes.gPyramid4Mesh.nIndices, GL_UNSIGNED_INT, (void*)0);
	glDrawElements(GL_TRIANGLES, meshes.gBoxMesh.nIndices, GL_UNSIGNED_INT, (void*)0);

	// Deactivate the VAO for box
	glBindVertexArray(0);
	/*          Plane          */
	glBindVertexArray(meshes.gPlaneMesh.vao);

	// Set the mesh transfomation values
	scale = glm::scale(glm::vec3(100.0f, 100.0f, 100.0f));
	rotation = glm::rotate(0.0f, glm::vec3(1.0f, 1.0f, 1.0f));
	translation = glm::translate(glm::vec3(0.0f, 0.0f, 0.0f));
	model = translation * rotation * scale;


	ubHasTextureVal = true;
	glUniform1i(ubHasTextureLoc, ubHasTextureVal);


	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, gTextureId8);


	// Remaining Object Specific Uniforms
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	glUniformMatrix4fv(objectColorLoc, 1, GL_FALSE, glm::value_ptr(model));
	glUniform1f(specularIntensity1Loc, 0.001f);
	glUniform1f(specularIntensity2Loc, 0.001f);
	glUniform1f(highlightSize1Loc, 50.0f);
	glUniform1f(highlightSize2Loc, 50.0f);

	glDrawElements(GL_TRIANGLES, meshes.gPlaneMesh.nIndices, GL_UNSIGNED_INT, (void*)0);

	// Deactivate the VAO for box
	glBindVertexArray(0);

	glfwSwapBuffers(gWindow);
}
//****************************************************
//  const char* vtxShaderSource: vertex shader source code
//  const char* fragShaderSource: fragment shader source code
//  GLuint &programId: unique ID of program associated with shaders
//****************************************************
bool CreateShaderProgram(const char* vtxShaderSource, const char* fragShaderSource, GLuint& programId)
{
	// Compilation and linkage error reporting
	int success = 0;
	char infoLog[512];

	// Create a Shader program object.
	programId = glCreateProgram();

	// Create the vertex and fragment shader objects
	GLuint vertexShaderId = glCreateShader(GL_VERTEX_SHADER);
	GLuint fragmentShaderId = glCreateShader(GL_FRAGMENT_SHADER);

	// Retrieve the shader source
	glShaderSource(vertexShaderId, 1, &vtxShaderSource, NULL);
	glShaderSource(fragmentShaderId, 1, &fragShaderSource, NULL);

	// Compile the vertex shader, and print compilation errors (if any)
	glCompileShader(vertexShaderId);
	// Check for vertex shader compile errors
	glGetShaderiv(vertexShaderId, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(vertexShaderId, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
		return false;
	}

	// Compile the fragment shader, and print compilation errors (if any)
	glCompileShader(fragmentShaderId);
	// Check for fragment shader compile errors
	glGetShaderiv(fragmentShaderId, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(fragmentShaderId, sizeof(infoLog), NULL, infoLog);
		std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
		return false;
	}

	// Attached compiled shaders to the shader program
	glAttachShader(programId, vertexShaderId);
	glAttachShader(programId, fragmentShaderId);

	// Links the shader program
	glLinkProgram(programId);
	// Check for linking errors
	glGetProgramiv(programId, GL_LINK_STATUS, &success);
	if (!success)
	{
		glGetProgramInfoLog(programId, sizeof(infoLog), NULL, infoLog);
		std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
		return false;
	}

	// Uses the shader program
	glUseProgram(programId);

	return true;
}
// Destroy the linked shader program //
void DestroyShaderProgram(GLuint programId)
{
	glDeleteProgram(programId);
}
// Images are loaded with Y axis going down, but OpenGL's Y axis goes up, so let's flip it //
void flipImageVertically(unsigned char* image, int width, int height, int channels)
{
	for (int j = 0; j < height / 2; ++j)
	{
		int index1 = j * width * channels;
		int index2 = (height - 1 - j) * width * channels;

		for (int i = width * channels; i > 0; --i)
		{
			unsigned char tmp = image[index1];
			image[index1] = image[index2];
			image[index2] = tmp;
			++index1;
			++index2;
		}
	}
}
// Generate and load the texture //
bool CreateTexture(const char* filename, GLuint& textureId)
{
	int width, height, channels;
	unsigned char* image = stbi_load(filename, &width, &height, &channels, 0);
	if (image)
	{
		flipImageVertically(image, width, height, channels);

		glGenTextures(1, &textureId);
		glBindTexture(GL_TEXTURE_2D, textureId);

		// set the texture wrapping parameters
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		// set texture filtering parameters
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		if (channels == 3)
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
		else if (channels == 4)
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
		else
		{
			cout << "Not implemented to handle image with " << channels << " channels" << endl;
			return false;
		}

		glGenerateMipmap(GL_TEXTURE_2D);

		stbi_image_free(image);
		glBindTexture(GL_TEXTURE_2D, 0); // Unbind the texture

		return true;
	}

	// Error loading the image
	return false;
}
// Release the texture attached to textureId //
void DestroyTexture(GLuint textureId)
{
	glGenTextures(1, &textureId);
}
void UMousePositionCallback(GLFWwindow* window, double xpos, double ypos) //callback for mouse x,y
{
	if (gFirstMouse)
	{
		gLastX = xpos;
		gLastY = ypos;
		gFirstMouse = false;
	}

	float xoffset = xpos - gLastX;
	float yoffset = gLastY - ypos; // reversed since y-coordinates go from bottom to top

	gLastX = xpos;
	gLastY = ypos;

	gCamera.ProcessMouseMovement(xoffset, yoffset);
}
void UMouseScrollCallback(GLFWwindow* window, double xoffset, double yoffset) //call back for scroll wheel
{
	gCamera.ProcessMouseScroll(yoffset);
}
void UMouseButtonCallback(GLFWwindow* window, int button, int action, int mods) //callback for clicks
{
	switch (button)
	{
	case GLFW_MOUSE_BUTTON_LEFT:
	{
		if (action == GLFW_PRESS)
			std::cout << "Left mouse button pressed" << std::endl;
		else
			std::cout << "Left mouse button released" << std::endl;
	}
	break;

	case GLFW_MOUSE_BUTTON_MIDDLE:
	{
		if (action == GLFW_PRESS)
			std::cout << "Middle mouse button pressed" << std::endl;
		else
			std::cout << "Middle mouse button released" << std::endl;
	}
	break;

	case GLFW_MOUSE_BUTTON_RIGHT:
	{
		if (action == GLFW_PRESS)
			std::cout << "Right mouse button pressed" << std::endl;
		else
			std::cout << "Right mouse button released" << std::endl;
	}
	break;

	default:
		std::cout << "Unhandled mouse button event" << std::endl;
		break;
	}
}