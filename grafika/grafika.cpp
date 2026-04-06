#define GLEW_DLL
#define GLFW_DLL
#include <iostream>
#include "GL\glew.h"
#include "GLFW/glfw3.h"
#include "shader.h"
#include "glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 3.0f);     
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);  
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);      


float yaw = -90.0f;         
float pitch = 0.0f;         
float lastX = 256.0f;       
float lastY = 256.0f;       
float mouseSensitivity = 0.1f;
bool firstMouse = true;

float cameraSpeed = 2.5f;

float deltaTime = 0.0f;
float lastFrame = 0.0f;

const unsigned int SCR_WIDTH = 1024;
const unsigned int SCR_HEIGHT = 768;

float vertices[] = {
    // Позиции          // Цвета
    -0.5f,  0.5f, 0.0f,  1.0f, 0.0f, 0.0f,  // верх-левый - красный
     0.5f,  0.5f, 0.0f,  0.0f, 1.0f, 0.0f,  // верх-правый - зеленый
     0.5f, -0.5f, 0.0f,  0.0f, 0.0f, 1.0f,  // низ-правый - синий
    -0.5f, -0.5f, 0.0f,  1.0f, 1.0f, 0.0f   // низ-левый - желтый
};

GLuint indices[] = {
    0, 1, 2,
    2, 3, 0
};

glm::vec3 cubePosition = glm::vec3(0.0f, 0.0f, 0.0f);


void processInput(GLFWwindow* window)
{
    float currentSpeed = cameraSpeed * deltaTime;

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        cameraPos += currentSpeed * cameraFront;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        cameraPos -= currentSpeed * cameraFront;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * currentSpeed;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * currentSpeed;
}

void mouse_callback(GLFWwindow* window, double xposIn, double yposIn)
{
    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);

    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;
    lastX = xpos;
    lastY = ypos;

    xoffset *= mouseSensitivity;
    yoffset *= mouseSensitivity;

    yaw += xoffset;
    pitch += yoffset;

    if (pitch > 89.0f)
        pitch = 89.0f;
    if (pitch < -89.0f)
        pitch = -89.0f;

    glm::vec3 front;
    front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    front.y = sin(glm::radians(pitch));
    front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    cameraFront = glm::normalize(front);
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

int main()
{
    if (!glfwInit()) {
        fprintf(stderr, "ERROR: could not start glfw. \n");
        return 1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window;
    window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "lab4", NULL, NULL);
    if (!window) {
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    glewExperimental = GL_TRUE;
    GLenum ret = glewInit();
    if (GLEW_OK != ret) {
        fprintf(stderr, "Error: %s\n", glewGetErrorString(ret));
        return 1;
    }

    GLuint VBO, VAO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);

    GLuint shaderProgram = createShaderProgram(
        "vertex.glsl",
        "fragment.glsl"
    );

    GLint modelLoc = glGetUniformLocation(shaderProgram, "model");
    GLint viewLoc = glGetUniformLocation(shaderProgram, "view");
    GLint projectionLoc = glGetUniformLocation(shaderProgram, "projection");

    //GLint transformLoc = glGetUniformLocation(shaderProgram, "transform");

    glEnable(GL_DEPTH_TEST);

    while (!glfwWindowShouldClose(window)) {

        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        processInput(window);

        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUseProgram(shaderProgram);

        glm::mat4 projection = glm::perspective(glm::radians(45.0f),
            (float)SCR_WIDTH / (float)SCR_HEIGHT,
            0.1f, 100.0f);


        glm::mat4 view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);

        glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

        glBindVertexArray(VAO);

            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, cubePosition);

            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteProgram(shaderProgram);

    glfwTerminate();
    return 0;
}
