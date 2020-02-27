#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Shader.h"
#include "Camera.h"

#include <iostream>
#include <vector>
#include <fstream>

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);

void processInput(GLFWwindow *window);
void initTextures();

// settings
const unsigned int SCR_WIDTH = 1920;
const unsigned int SCR_HEIGHT = 1080;

// File has only image data. The dimension of the data should be known.
int m_uImageCount = 109;
int m_uImageWidth = 256;
int m_uImageHeight = 256;

GLuint m_puTextureID;

Camera camera(glm::vec3(0.0f, 1.0f, 3.0f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

float deltaTime = 0.0f;
float lastFrame = 0.0f;

int main()
{
	// glfw: initialize and configure
	// ------------------------------
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Volume Rendering", NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);

	// tell GLFW to capture our mouse
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	glfwSwapInterval(0);

	// glad: load all OpenGL function pointers
	// ---------------------------------------
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

	std::vector<float>* data = new std::vector<float>();
	data->resize((size_t)(24) * (size_t)(m_uImageCount));

	std::vector<GLuint>* dataIndex = new std::vector<GLuint>();
	dataIndex->resize((size_t)(6) * (size_t)(m_uImageCount));

	int i = 0, iIndex = 0, lIndex = 0;
	for (int j=0; j<m_uImageCount; j++)
	{
		float mappedZVal = -1.0f + 2.0f * (float)j / 109.0f;

		float zTex = (float) mappedZVal + 1.0f / 2.0f;

		// QUAD DATA
		(*data)[i++] = 0.5f; (*data)[i++] = 0.5f; (*data)[i++] = mappedZVal;
		(*data)[i++] = 1.0f; (*data)[i++] = 1.0f; (*data)[i++] = zTex;

		(*data)[i++] = 0.5f; (*data)[i++] = -0.5f; (*data)[i++] = mappedZVal;
		(*data)[i++] = 1.0f; (*data)[i++] = 0.0f;  (*data)[i++] = zTex;

		(*data)[i++] = -0.5f; (*data)[i++] = -0.5f; (*data)[i++] = mappedZVal;
		(*data)[i++] = 0.0f;  (*data)[i++] = 0.0f;  (*data)[i++] = zTex;

		(*data)[i++] = -0.5f; (*data)[i++] = 0.5f; (*data)[i++] = mappedZVal;
		(*data)[i++] = 0.0f;  (*data)[i++] = 1.0f; (*data)[i++] = zTex;

		// QUAD DATA INDICES
		(*dataIndex)[lIndex++] = iIndex;     (*dataIndex)[lIndex++] = iIndex + 1; (*dataIndex)[lIndex++] = iIndex + 3; // First triangle
		(*dataIndex)[lIndex++] = iIndex + 1; (*dataIndex)[lIndex++] = iIndex + 2; (*dataIndex)[lIndex++] = iIndex + 3; // Second triangle
		iIndex += 4;
	}

	unsigned int VAO, VBO, EBO;
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);

	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, data->size() * sizeof(float), &(*data)[0], GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, dataIndex->size() * sizeof(float), &(*dataIndex)[0], GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	glEnable(GL_DEPTH_TEST);
	//glDisable(GL_CULL_FACE);

	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
	glBlendEquation(GL_MAX);

	Shader sliceShader("Shaders/vertex.glsl", "Shaders/fragment.glsl");

	initTextures();

	while (!glfwWindowShouldClose(window))
	{
		float currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		std::stringstream ss;
		ss << "Volume rendering " << deltaTime * 1000 << " ms";

		glfwSetWindowTitle(window, ss.str().c_str());
		//std::cout << "Time per frame: " << deltaTime * 1000 << " ms" << std::endl;

		processInput(window);

		glClearColor(0.05f, 0.05f, 0.05f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// bind Texture
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_3D, m_puTextureID);

		// render container
		glm::mat4 sliceView = camera.GetViewMatrix();
		glm::mat4 sliceProj = glm::perspective(camera.Zoom, (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);

		glm::mat4 sliceModel = glm::mat4(1.0f);
		sliceModel = glm::translate(sliceModel, glm::vec3(0.0f, 0.0f, 0.0f));
		sliceModel = glm::rotate(sliceModel, glm::radians(180.0f), glm::vec3(1.0f, 0.0f, 0.0f));
		sliceModel = glm::rotate(sliceModel, (float)glfwGetTime(), glm::vec3(0.0f, 1.0f, 0.0f));
		sliceModel = glm::scale(sliceModel, glm::vec3(2.0f, 2.0f, 1.0f));

		sliceShader.use();
		sliceShader.setMat4("projection", sliceProj);
		sliceShader.setMat4("view", sliceView);
		sliceShader.setMat4("model", sliceModel);
		sliceShader.setInt("ourTexture", 0);

		glDrawElements(GL_TRIANGLES, 6 * m_uImageCount, GL_UNSIGNED_INT, 0);

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glfwTerminate();
	return 0;
}

void processInput(GLFWwindow *window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		camera.ProcessKeyboard(FORWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		camera.ProcessKeyboard(BACKWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		camera.ProcessKeyboard(LEFT, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		camera.ProcessKeyboard(RIGHT, deltaTime);
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	if (firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

	lastX = xpos;
	lastY = ypos;

	camera.ProcessMouseMovement(xoffset, yoffset);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	//camera.ProcessMouseScroll(yoffset);
}

void initTextures()
{
	std::ifstream file("head256x256x109", std::ifstream::binary);

	std::string fileName;

	// Holds the luminance buffer
	char* chBuffer = new char[m_uImageWidth * m_uImageHeight * m_uImageCount];
	char* chRGBABuffer = new char[m_uImageWidth * m_uImageHeight * m_uImageCount * 4];

	file.read(chBuffer, m_uImageWidth * m_uImageHeight * m_uImageCount);

	glGenTextures(1, &m_puTextureID);
	glBindTexture(GL_TEXTURE_3D, m_puTextureID);

	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);


	for (int nIndx = 0; nIndx < m_uImageWidth * m_uImageHeight * m_uImageCount; ++nIndx)
	{
		chRGBABuffer[nIndx * 4] = chBuffer[nIndx];
		chRGBABuffer[nIndx * 4 + 1] = chBuffer[nIndx];
		chRGBABuffer[nIndx * 4 + 2] = chBuffer[nIndx];
		chRGBABuffer[nIndx * 4 + 3] = 255;
		if (chBuffer[nIndx] < 20)
		{
			chRGBABuffer[nIndx * 4 + 3] = 0;
		}
	}

	glTexImage3D(GL_TEXTURE_3D, 0, GL_RGBA, m_uImageWidth, m_uImageHeight, m_uImageCount, 0,
		GL_RGBA, GL_UNSIGNED_BYTE, (GLvoid*)chRGBABuffer);
	glBindTexture(GL_TEXTURE_3D, 0);

	std::cout << "Loaded texture data" << std::endl;

	delete[] chBuffer;
	delete[] chRGBABuffer;
}