//
// Created by Yohanes on 29/10/2024.
//
// gl_renderer.h
#ifndef OPENGLJAVA_GL_RENDERER_H
#define OPENGLJAVA_GL_RENDERER_H

#pragma once
#include <jni.h>
#include <GLES2/gl2.h>
#include <string>
#include <unordered_map>
#include <vector>

class Renderer {
private:
    GLuint loadShader(GLenum type, const char* source);
    std::unordered_map<std::string, GLuint> shaderPrograms;
    float projectionMatrix[16];
    float viewMatrix[16];
    float mvpMatrix[16];
    // Helper functions for matrix operations
    void setLookAt(float eyeX, float eyeY, float eyeZ,
                   float centerX, float centerY, float centerZ,
                   float upX, float upY, float upZ);
    void frustum(float left, float right, float bottom, float top,
                 float near, float far);
    void multiplyMM(float* result, const float* lhs, const float* rhs);

public:
    void init();
    void setupView(int width, int height);
    GLuint getOrCreateShaderProgram(const std::string& vertexShaderPath,
                                    const std::string& fragmentShaderPath);
    void drawShape(const std::string& programName,
                   const float* vertices, int vertexCount,
                   const float* texCoords,
                   const float* modelMatrix);
};


#endif //OPENGLJAVA_GL_RENDERER_H
