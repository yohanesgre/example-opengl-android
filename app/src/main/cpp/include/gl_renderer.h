//
// Created by Yohanes on 29/10/2024.
//
// gl_renderer.h
#ifndef OPENGLJAVA_GL_RENDERER_H
#define OPENGLJAVA_GL_RENDERER_H
#pragma once

#include <GLES2/gl2.h>
#include <string>
#include <map>
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

class Renderer {
public:
    void init();
    void setAssetManager(AAssetManager* mgr);
    void setupView(int width, int height);
    GLuint getOrCreateShaderProgram(const std::string& vertexPath, const std::string& fragmentPath);
    void drawShape(const std::string& programName,
                   const float* vertices, int vertexCount,
                   const float* texCoords,
                   const float* modelMatrix,
                   float scaleX, float scaleY, float scaleZ,
                   float rotationAngle, float rotationX, float rotationY, float rotationZ);

private:
    AAssetManager* assetManager;
    std::string loadFileContents(const std::string& filePath);
    GLuint loadShader(GLenum type, const char* source);
    std::map<std::string, GLuint> shaderPrograms;


    // Replace float arrays with GLM matrices
    glm::mat4 projectionMatrix;
    glm::mat4 viewMatrix;
    glm::mat4 mvpMatrix;
};


#endif //OPENGLJAVA_GL_RENDERER_H
