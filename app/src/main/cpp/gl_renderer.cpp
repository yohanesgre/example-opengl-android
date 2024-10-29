// gl_renderer.cpp
#include "include/gl_renderer.h"
#include "include/matrix.h"
#include <fstream>
#include <sstream>
#include <android/log.h>

#define LOG_TAG "NativeRenderer"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

static void logMatrix(const char* name, const float* m) {
    LOGI("Matrix %s:", name);
    for (int i = 0; i < 4; i++) {
        LOGI("Row %d: %.2f, %.2f, %.2f, %.2f",
             i, m[i*4], m[i*4+1], m[i*4+2], m[i*4+3]);
    }
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

    // Initialize matrices
    Matrix::setIdentityM(projectionMatrix, 0);
    Matrix::setIdentityM(viewMatrix, 0);
    Matrix::setIdentityM(mvpMatrix, 0);

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
    float ratio = (float)height / width;

    LOGI("Setting up view with dimensions: %dx%d, ratio: %f", width, height, ratio);

    // Use a closer near plane and adjusted far plane
    const float near = 5.0f;
    const float far = 10.0f;
    const float left = -1.0f;
    const float right = 1.0f;
    const float bottom = -ratio;
    const float top = ratio;

    // Create projection matrix
    Matrix::frustumM(projectionMatrix, 0,
                     left, right,
                     bottom, top,
                     near, far);

    logMatrix("Projection after frustum", projectionMatrix);

    // Position camera slightly back and up for better view
    const float eyeX = 0.0f;
    const float eyeY = 0.0f;
    const float eyeZ = 5.0f;  // Moved closer
    const float lookX = 0.0f;
    const float lookY = 0.0f;
    const float lookZ = 0.0f;
    const float upX = 0.0f;
    const float upY = 1.0f;
    const float upZ = 0.0f;

    // Create view matrix
    Matrix::setLookAtM(viewMatrix, 0,
                       eyeX, eyeY, eyeZ,
                       lookX, lookY, lookZ,
                       upX, upY, upZ);

    logMatrix("View after lookAt", viewMatrix);

    // Calculate view-projection matrix
    Matrix::multiplyMM(mvpMatrix, 0, projectionMatrix, 0, viewMatrix, 0);

    logMatrix("MVP after projection * view", mvpMatrix);
}

void Renderer::drawShape(const std::string& programName,
                         const float* vertices, int vertexCount,
                         const float* texCoords,
                         const float* modelMatrix) {
    LOGI("Drawing shape with %d vertices", vertexCount);
    logMatrix("Input Model Matrix", modelMatrix);

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

    // Set up vertex buffers
    GLuint vbo[2];
    glGenBuffers(2, vbo);

    // Upload vertex data
    glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
    glBufferData(GL_ARRAY_BUFFER, vertexCount * 3 * sizeof(float), vertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(positionHandle);
    glVertexAttribPointer(positionHandle, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

    // Log first vertex for debugging
    if (vertices && vertexCount > 0) {
        LOGI("First vertex: (%.2f, %.2f, %.2f)",
             vertices[0], vertices[1], vertices[2]);
    }

    // Upload texture coordinates
    glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
    glBufferData(GL_ARRAY_BUFFER, vertexCount * 2 * sizeof(float), texCoords, GL_STATIC_DRAW);
    glEnableVertexAttribArray(texCoordHandle);
    glVertexAttribPointer(texCoordHandle, 2, GL_FLOAT, GL_FALSE, 0, nullptr);

    // Calculate final MVP matrix
    float tempMVP[16];
    Matrix::multiplyMM(tempMVP, 0, mvpMatrix, 0, modelMatrix, 0);

    logMatrix("Final MVP Matrix", tempMVP);

    // Set MVP matrix uniform
    glUniformMatrix4fv(mvpMatrixHandle, 1, GL_FALSE, tempMVP);

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