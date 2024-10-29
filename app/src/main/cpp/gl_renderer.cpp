#include "include/gl_renderer.h"
#include <fstream>
#include <sstream>
#include <android/log.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>

#define LOG_TAG "NativeRenderer"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

static void logMatrix(const char* name, const glm::mat4& m) {
    std::string matStr = glm::to_string(m);
    LOGI("Matrix %s:\n%s", name, matStr.c_str());
}

void checkShaderCompileStatus(GLuint shader) {
    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        GLchar infoLog[512];
        glGetShaderInfoLog(shader, sizeof(infoLog), NULL, infoLog);
        LOGE("Shader compilation failed: %s", infoLog);
    }
}

void checkProgramLinkStatus(GLuint program) {
    GLint success;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        GLchar infoLog[512];
        glGetProgramInfoLog(program, sizeof(infoLog), NULL, infoLog);
        LOGE("Program linking failed: %s", infoLog);
    }
}

void Renderer::init() {
    glEnable(GL_DEPTH_TEST);
    glClearColor(0.2f, 0.2f, 0.2f, 1.0f);

    // Initialize matrices using GLM identity
    projectionMatrix = glm::mat4(1.0f);
    viewMatrix = glm::mat4(1.0f);
    mvpMatrix = glm::mat4(1.0f);

    logMatrix("Initial Projection", projectionMatrix);
    logMatrix("Initial View", viewMatrix);
    logMatrix("Initial MVP", mvpMatrix);

    LOGI("Renderer initialized");
}

GLuint Renderer::loadShader(GLenum type, const char* source) {
    GLuint shader = glCreateShader(type);
    if (shader == 0) {
        LOGE("Failed to create shader of type %d", type);
        return 0;
    }

    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);
    checkShaderCompileStatus(shader);

    return shader;
}

GLuint Renderer::getOrCreateShaderProgram(const std::string& vertexPath,
                                          const std::string& fragmentPath) {
    std::string key = vertexPath + ":" + fragmentPath;
    auto it = shaderPrograms.find(key);
    if (it != shaderPrograms.end()) {
        return it->second;
    }

    const char* vertexShaderSource =
            "uniform mat4 uMVPMatrix;\n"
            "attribute vec4 vPosition;\n"
            "attribute vec2 texCoord;\n"
            "varying vec2 uv;\n"
            "void main() {\n"
            "    gl_Position = uMVPMatrix * vPosition;\n"
            "    uv = texCoord;\n"
            "}\n";

    const char* fragmentShaderSource =
            "precision mediump float;\n"
            "varying vec2 uv;\n"
            "void main() {\n"
            "    vec3 barycentric = vec3(1.0 - uv.x - uv.y, uv.x, uv.y);\n"
            "    vec3 color = barycentric.x * vec3(1.0, 0.0, 0.0) +\n"
            "                 barycentric.y * vec3(0.0, 1.0, 0.0) +\n"
            "                 barycentric.z * vec3(0.0, 0.0, 1.0);\n"
            "    gl_FragColor = vec4(color, 1.0);\n"
            "}\n";

    GLuint vertexShader = loadShader(GL_VERTEX_SHADER, vertexShaderSource);
    GLuint fragmentShader = loadShader(GL_FRAGMENT_SHADER, fragmentShaderSource);

    if (!vertexShader || !fragmentShader) {
        LOGE("Failed to create shaders");
        return 0;
    }

    GLuint program = glCreateProgram();
    if (program == 0) {
        LOGE("Failed to create program");
        return 0;
    }

    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);
    glLinkProgram(program);
    checkProgramLinkStatus(program);

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    shaderPrograms[key] = program;
    return program;
}

void Renderer::setupView(int width, int height) {
    glViewport(0, 0, width, height);
    float aspectRatio = static_cast<float>(width) / static_cast<float>(height);

    LOGI("Setting up view with dimensions: %dx%d, aspect ratio: %f",
         width, height, aspectRatio);

    // Create perspective projection matrix
    float fovY = glm::radians(45.0f);
    float nearPlane = 0.1f;
    float farPlane = 100.0f;
    projectionMatrix = glm::perspective(fovY, aspectRatio, nearPlane, farPlane);
    logMatrix("Projection Matrix", projectionMatrix);

    // Setup camera view matrix
    glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 20.0f);
    glm::vec3 cameraTarget = glm::vec3(0.0f, 0.0f, 0.0f);
    glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
    viewMatrix = glm::lookAt(cameraPos, cameraTarget, cameraUp);
    logMatrix("View Matrix", viewMatrix);

    // Compute view-projection matrix
    mvpMatrix = projectionMatrix * viewMatrix;
    logMatrix("MVP Matrix", mvpMatrix);
}

void Renderer::drawShape(const std::string& programName,
                         const float* vertices, int vertexCount,
                         const float* texCoords,
                         const float* modelMatrix,
                         float scaleX, float scaleY, float scaleZ,
                         float rotationAngle, float rotationX, float rotationY, float rotationZ) {
    LOGI("Drawing shape with %d vertices", vertexCount);
    LOGI("Scale: (%.2f, %.2f, %.2f)", scaleX, scaleY, scaleZ);
    LOGI("Rotation: %.2f degrees around (%.2f, %.2f, %.2f)",
         rotationAngle, rotationX, rotationY, rotationZ);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    GLuint program = shaderPrograms[programName];
    glUseProgram(program);

    // Get shader locations
    GLint positionHandle = glGetAttribLocation(program, "vPosition");
    GLint texCoordHandle = glGetAttribLocation(program, "texCoord");
    GLint mvpMatrixHandle = glGetUniformLocation(program, "uMVPMatrix");

    if (positionHandle < 0 || texCoordHandle < 0 || mvpMatrixHandle < 0) {
        LOGE("Failed to get shader locations");
        return;
    }

    // Calculate the center of the shape
    glm::vec3 center(0.0f);
    for (int i = 0; i < vertexCount * 3; i += 3) {
        center.x += vertices[i];
        center.y += vertices[i + 1];
        center.z += vertices[i + 2];
    }
    center /= static_cast<float>(vertexCount);

    // Build model matrix using GLM
    glm::mat4 model = glm::mat4(1.0f);

    // Extract translation from input model matrix
    glm::vec3 translation(modelMatrix[12], modelMatrix[13], modelMatrix[14]);

    // Order of transformations:
    // 1. Translate to origin (center)
    // 2. Rotate
    // 3. Scale
    // 4. Translate back
    model = glm::translate(model, translation);                                    // Final position
    model = glm::translate(model, center);                                        // Move back from origin
    model = glm::scale(model, glm::vec3(scaleX, scaleY, scaleZ));               // Apply scale
    model = glm::rotate(model,
                        glm::radians(rotationAngle),
                        glm::normalize(glm::vec3(rotationX, rotationY, rotationZ))); // Apply rotation
    model = glm::translate(model, -center);                                       // Move to origin

    logMatrix("Model Matrix", model);

    // Calculate final MVP matrix
    glm::mat4 finalMVP = mvpMatrix * model;
    logMatrix("Final MVP Matrix", finalMVP);

    // Set up vertex buffers
    GLuint vbo[2];
    glGenBuffers(2, vbo);

    // Upload vertex data
    glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
    glBufferData(GL_ARRAY_BUFFER, vertexCount * 3 * sizeof(float),
                 vertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(positionHandle);
    glVertexAttribPointer(positionHandle, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

    // Upload texture coordinates
    glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
    glBufferData(GL_ARRAY_BUFFER, vertexCount * 2 * sizeof(float),
                 texCoords, GL_STATIC_DRAW);
    glEnableVertexAttribArray(texCoordHandle);
    glVertexAttribPointer(texCoordHandle, 2, GL_FLOAT, GL_FALSE, 0, nullptr);

    // Set MVP matrix uniform using GLM
    glUniformMatrix4fv(mvpMatrixHandle, 1, GL_FALSE, glm::value_ptr(finalMVP));

    // Draw triangles
    glDrawArrays(GL_TRIANGLES, 0, vertexCount);

    // Check for errors
    GLenum error = glGetError();
    if (error != GL_NO_ERROR) {
        LOGE("GL error after drawing: 0x%x", error);
    }

    // Cleanup
    glDisableVertexAttribArray(positionHandle);
    glDisableVertexAttribArray(texCoordHandle);
    glDeleteBuffers(2, vbo);
}