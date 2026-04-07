#define GLEW_DLL
#define GLFW_DLL
#include <iostream>
#include "GL\glew.h"
#include "GLFW/glfw3.h"
#include "shader.h"
#include "glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include <vector>
#include <string>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

using namespace std;


glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 10.0f);     
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


float angleOX1 = 0.0f;
float angleOX2 = 0.0f;
float angleOX3 = 0.0f;

float scaleValue = 1.0f;

glm::vec3 cubePosition = glm::vec3(0.0f, 0.0f, 0.0f);

struct Vertex {
    glm::vec3 Position;
    glm::vec3 Normal;
};

class Mesh {
public:
    vector<Vertex> vertices;
    vector<unsigned int> indices;

    string name;

    GLuint VAO, VBO, EBO;

    Mesh(vector<Vertex> vertices, vector<unsigned int> indices, string name = "") {
        this->vertices = vertices;
        this->indices = indices;
        this->name = name;
        setupMesh();
    }

    void Draw() {
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
    }

private:
    void setupMesh() {
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glGenBuffers(1, &EBO);

        glBindVertexArray(VAO);

        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
        glEnableVertexAttribArray(0);

        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Normal));
        glEnableVertexAttribArray(1);

        glBindVertexArray(0);
    }
};

class Model {
public:
    vector<Mesh> meshes;

    Model(string path) {
        loadModel(path);
    }

    void Draw(GLuint shaderProgram,
        glm::mat4 r1,
        glm::mat4 r2,
        glm::mat4 r3)
    { 
        glm::vec3 pivot1(0, 0, 2.40817);
        glm::vec3 pivot2(0, 1.158, 0.9);
        glm::vec3 pivot3(0, 0.876685, -2.17901);

        glm::mat4 base = glm::mat4(1.0f);
        base = glm::scale(base, glm::vec3(scaleValue));

        //  вращается вокруг pivot1
        glm::mat4 transform1 = base;
        transform1 = glm::translate(transform1, pivot1);
        transform1 = transform1 * r1;
        transform1 = glm::translate(transform1, -pivot1);

        //вращается вокруг pivot2
        glm::mat4 transform2 = transform1;
        transform2 = glm::translate(transform2, pivot2);
        transform2 = transform2 * r2;
        transform2 = glm::translate(transform2, -pivot2);

        // вращается вокруг pivot3
        glm::mat4 transform3 = transform2;
        transform3 = glm::translate(transform3, pivot3);
        transform3 = transform3 * r3;
        transform3 = glm::translate(transform3, -pivot3);

        for (unsigned int i = 0; i < meshes.size(); i++) {
            glm::mat4 model = glm::mat4(1.0f);

            if (meshes[i].name.find("Link3") != string::npos ||
                meshes[i].name.find("joint3") != string::npos) {
                model = transform3;
            }
            else if (meshes[i].name.find("Link2") != string::npos ||
                meshes[i].name.find("joint2") != string::npos) {
                model = transform2;
            }
            else if (meshes[i].name.find("Link1") != string::npos ||
                meshes[i].name.find("joint1") != string::npos) {
                model = transform1;
            }
            else {
                model = base;
            }

            glUniformMatrix4fv(
                glGetUniformLocation(shaderProgram, "model"),
                1, GL_FALSE,
                glm::value_ptr(model)
            );

            meshes[i].Draw();
        }
    }

private:
    void loadModel(string path) {
        Assimp::Importer importer;
        const aiScene* scene = importer.ReadFile(path,aiProcess_Triangulate | aiProcess_GenNormals);


        processNode(scene->mRootNode, scene);
    }

    void processNode(aiNode* node, const aiScene* scene) {
        for (unsigned int i = 0; i < node->mNumMeshes; i++) {
            aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
            meshes.push_back(processMesh(mesh));
        }

        for (unsigned int i = 0; i < node->mNumChildren; i++) {
            processNode(node->mChildren[i], scene);
        }
    }

    Mesh processMesh(aiMesh* mesh) {
        vector<Vertex> vertices;
        vector<unsigned int> indices;

        for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
            Vertex vertex;

            vertex.Position = glm::vec3(
                mesh->mVertices[i].x,
                mesh->mVertices[i].y,
                mesh->mVertices[i].z
            );

            vertex.Normal = glm::vec3(
                mesh->mNormals[i].x,
                mesh->mNormals[i].y,
                mesh->mNormals[i].z
            );

            vertices.push_back(vertex);
        }

        for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
            aiFace face = mesh->mFaces[i];
            for (unsigned int j = 0; j < face.mNumIndices; j++) {
                indices.push_back(face.mIndices[j]);
            }
        }

        return Mesh(vertices, indices, mesh->mName.C_Str());
    }
};



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

    int rotCoeff = 50;

    if (glfwGetKey(window, GLFW_KEY_N) == GLFW_PRESS)
        angleOX1 -= deltaTime * rotCoeff;

    if (glfwGetKey(window, GLFW_KEY_M) == GLFW_PRESS)
        angleOX1 += deltaTime * rotCoeff;

    if (glfwGetKey(window, GLFW_KEY_J) == GLFW_PRESS && angleOX2 < 110)
        angleOX2 += deltaTime * rotCoeff;

    if (glfwGetKey(window, GLFW_KEY_K) == GLFW_PRESS && angleOX2 > -70)
        angleOX2 -= deltaTime * rotCoeff;

    if (glfwGetKey(window, GLFW_KEY_I) == GLFW_PRESS && angleOX3 < 150)
        angleOX3 += deltaTime * rotCoeff;

    if (glfwGetKey(window, GLFW_KEY_O) == GLFW_PRESS && angleOX3 > -35)
        angleOX3 -= deltaTime * rotCoeff;
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


    GLuint shaderProgram = createShaderProgram(
        "vertex.glsl",
        "fragment.glsl"
    );

    GLint modelLoc = glGetUniformLocation(shaderProgram, "model");
    GLint viewLoc = glGetUniformLocation(shaderProgram, "view");
    GLint projectionLoc = glGetUniformLocation(shaderProgram, "projection");

    glEnable(GL_DEPTH_TEST);


    Model myModel("model/bidulyaObj.obj");
    while (!glfwWindowShouldClose(window)) {

        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        processInput(window);

        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUseProgram(shaderProgram);

        glUniform3f(glGetUniformLocation(shaderProgram, "light.ambient"), 0.2f, 0.2f, 0.2f);

        glUniform3f(glGetUniformLocation(shaderProgram, "light.diffuse"), 0.8f, 0.8f, 0.8f);

        glUniform3f(glGetUniformLocation(shaderProgram, "light.specular"), 1.0f, 1.0f, 1.0f);

        glUniform3f(glGetUniformLocation(shaderProgram, "light.position"), 1.2f, 0.0f, 4.0f);

        glUniform3f(glGetUniformLocation(shaderProgram, "viewPos"), cameraPos.x, cameraPos.y, cameraPos.z);

        glUniform3f(glGetUniformLocation(shaderProgram, "material.ambient"), 1.0f, 0.5f, 0.31f);

        glUniform3f(glGetUniformLocation(shaderProgram, "material.diffuse"), 1.0f, 0.5f, 0.31f);

        glUniform3f(glGetUniformLocation(shaderProgram, "material.specular"),0.5f, 0.5f, 0.5f);

        glUniform1f(glGetUniformLocation(shaderProgram, "material.shininess"), 32.0f);
        glm::mat4 projection = glm::perspective(glm::radians(45.0f),
            (float)SCR_WIDTH / (float)SCR_HEIGHT,
            0.1f, 100.0f);


        glm::mat4 view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
        glm::mat4 model = glm::mat4(1.0f);

        model = glm::scale(model, glm::vec3(scaleValue));

        glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

        glm::mat4 r1 = glm::rotate(glm::mat4(1.0f),
            glm::radians(angleOX1),
            glm::vec3(0, 1, 0));

        glm::mat4 r2 = glm::rotate(glm::mat4(1.0f),
            glm::radians(angleOX2),
            glm::vec3(1, 0, 0));

        glm::mat4 r3 = glm::rotate(glm::mat4(1.0f),
            glm::radians(angleOX3),
            glm::vec3(1, 0, 0));

        myModel.Draw(shaderProgram, r1, r2, r3);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }


    glDeleteProgram(shaderProgram);

    glfwTerminate();
    return 0;
}
