#define GLEW_DLL
#define GLFW_DLL
#include <iostream>
#include "GL\glew.h"
#include "GLFW/glfw3.h"
void StartWindow()
{

}
int main()
{
	if (!glfwInit()) {
		fprintf(stderr, "ERROR: could not start glfw. \n");
		return 1;
	}
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 1);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

	GLFWwindow* window;
	window = glfwCreateWindow(512, 512, "Mainwindow", NULL, NULL);
	if (!window) {
		glfwTerminate();
		return -1;
	}

	glfwMakeContextCurrent(window);
	glewExperimental = GL_TRUE;

	GLenum ret = glewInit();
	if (GLEW_OK != ret) {
		fprintf(stderr, "Error :%s\n", glewGetErrorString(ret));
		return 1;
	}

	while (!glfwWindowShouldClose(window)) {
		glClearColor(0.9f, 0.1f, 0.2f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		glBegin(GL_QUADS);

		glColor3f(0.1f, 0.2f, 0.8f);

		glVertex2f(-0.5f, 0.5f); // левый верх
		glVertex2f(0.5f, 0.5f); // правый верх
		glVertex2f(0.5f, -0.5f); // правый низ
		glVertex2f(-0.5f, -0.5f); // левый низ

		glEnd();

		glfwSwapBuffers(window);
		glfwPollEvents();
	}
	glfwTerminate();
	return 1;

}

