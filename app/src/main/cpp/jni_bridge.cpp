// jni_bridge.cpp
#include <jni.h>
#include "include/gl_renderer.h"
#include <android/log.h>
#include <stdexcept>
#include <memory>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#define LOG_TAG "JNIBridge"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

// Global renderer instance using smart pointer
static std::unique_ptr<Renderer> renderer;

// Helper function to check for GL errors
static void checkGLError(const char* operation) {
    GLenum error;
    while ((error = glGetError()) != GL_NO_ERROR) {
        LOGE("GL Error after %s: 0x%x", operation, error);
    }
}

// RAII wrapper for JNI float arrays with size information
class JFloatArrayGuard {
public:
    JFloatArrayGuard(JNIEnv* env, jfloatArray array)
            : env(env), array(array), ptr(nullptr), size(0) {
        if (array) {
            size = env->GetArrayLength(array);
            ptr = env->GetFloatArrayElements(array, nullptr);
        }
    }

    ~JFloatArrayGuard() {
        if (ptr && array) {
            env->ReleaseFloatArrayElements(array, ptr, JNI_ABORT);
        }
    }

    jfloat* get() { return ptr; }
    size_t getSize() const { return size; }
    operator bool() const { return ptr != nullptr; }

private:
    JNIEnv* env;
    jfloatArray array;
    jfloat* ptr;
    size_t size;
};

// RAII wrapper for JNI strings
class JStringGuard {
public:
    JStringGuard(JNIEnv* env, jstring str)
            : env(env), str(str), ptr(nullptr) {
        if (str) {
            ptr = env->GetStringUTFChars(str, nullptr);
        }
    }

    ~JStringGuard() {
        if (ptr && str) {
            env->ReleaseStringUTFChars(str, ptr);
        }
    }

    const char* get() { return ptr; }
    operator bool() const { return ptr != nullptr; }

private:
    JNIEnv* env;
    jstring str;
    const char* ptr;
};

// Helper function to validate transformation parameters
static bool validateTransformParams(float scaleX, float scaleY, float scaleZ,
                                    float rotationAngle, float rotationX, float rotationY, float rotationZ) {
    // Check for invalid scale values
    if (scaleX == 0.0f || scaleY == 0.0f || scaleZ == 0.0f) {
        LOGE("Invalid scale values: (%f, %f, %f)", scaleX, scaleY, scaleZ);
        return false;
    }

    // Check if rotation axis is zero vector
    if (rotationX == 0.0f && rotationY == 0.0f && rotationZ == 0.0f) {
        LOGE("Invalid rotation axis: (0, 0, 0)");
        return false;
    }

    return true;
}

extern "C" {

JNIEXPORT void JNICALL
Java_com_example_opengljava_NativeRenderer_init(JNIEnv* env, jobject /* obj */) {
    try {
        if (!renderer) {
            renderer = std::make_unique<Renderer>();
            renderer->init();
            LOGI("Renderer initialized successfully");
            checkGLError("init");
        } else {
            LOGI("Renderer already initialized");
        }
    } catch (const std::exception& e) {
        LOGE("Error in init: %s", e.what());
        renderer.reset();
    }
}

JNIEXPORT void JNICALL
Java_com_example_opengljava_NativeRenderer_drawShape(
        JNIEnv* env, jobject /* obj */,
        jfloatArray vertices, jfloatArray texCoords, jfloatArray modelMatrix,
        jstring vertexShader, jstring fragmentShader,
        jfloat scaleX, jfloat scaleY, jfloat scaleZ,
        jfloat rotationAngle, jfloat rotationX, jfloat rotationY, jfloat rotationZ) {

    if (!renderer) {
        LOGE("Renderer not initialized");
        return;
    }

    try {
        // Validate transformation parameters
        if (!validateTransformParams(scaleX, scaleY, scaleZ,
                                     rotationAngle, rotationX, rotationY, rotationZ)) {
            throw std::runtime_error("Invalid transformation parameters");
        }

        // Use RAII guards for array access
        JFloatArrayGuard vertexPtr(env, vertices);
        JFloatArrayGuard texCoordPtr(env, texCoords);
        JFloatArrayGuard modelMatrixPtr(env, modelMatrix);
        JStringGuard vertexShaderPath(env, vertexShader);
        JStringGuard fragmentShaderPath(env, fragmentShader);

        if (!vertexPtr || !texCoordPtr || !modelMatrixPtr ||
            !vertexShaderPath || !fragmentShaderPath) {
            throw std::runtime_error("Failed to access JNI arrays or strings");
        }

        // Validate array sizes
        size_t vertexCount = vertexPtr.getSize() / 3;
        if (vertexCount == 0 || vertexPtr.getSize() % 3 != 0) {
            throw std::runtime_error("Invalid vertex data");
        }

        if (texCoordPtr.getSize() != vertexCount * 2) {
            throw std::runtime_error("Texture coordinate data size mismatch");
        }

        if (modelMatrixPtr.getSize() != 16) {
            throw std::runtime_error("Invalid model matrix size");
        }

        // Log transformation parameters
        LOGI("Transform params - Scale: (%.2f, %.2f, %.2f), Rotation: %.2f° around (%.2f, %.2f, %.2f)",
             scaleX, scaleY, scaleZ, rotationAngle, rotationX, rotationY, rotationZ);

        // Create shader program key and get/create program
        std::string programKey = std::string(vertexShaderPath.get()) + ":" +
                                 fragmentShaderPath.get();

        GLuint program = renderer->getOrCreateShaderProgram(
                vertexShaderPath.get(),
                fragmentShaderPath.get()
        );

        if (program == 0) {
            throw std::runtime_error("Failed to create or get shader program");
        }

        // Draw the shape
        renderer->drawShape(
                programKey,
                vertexPtr.get(),
                static_cast<int>(vertexCount),
                texCoordPtr.get(),
                modelMatrixPtr.get(),
                scaleX, scaleY, scaleZ,
                rotationAngle,
                rotationX, rotationY, rotationZ
        );

        checkGLError("drawShape");

    } catch (const std::exception& e) {
        LOGE("Error in drawShape: %s", e.what());
    }
}

JNIEXPORT void JNICALL
Java_com_example_opengljava_NativeRenderer_setupView(
        JNIEnv* /* env */, jobject /* obj */,
        jint width, jint height) {

    if (!renderer) {
        LOGE("Renderer not initialized");
        return;
    }

    try {
        if (width <= 0 || height <= 0) {
            throw std::runtime_error("Invalid dimensions");
        }

        renderer->setupView(width, height);
        checkGLError("setupView");
        LOGI("View setup complete: %dx%d", width, height);
    } catch (const std::exception& e) {
        LOGE("Error in setupView: %s", e.what());
    }
}

JNIEXPORT void JNICALL
Java_com_example_opengljava_NativeRenderer_cleanup(JNIEnv* /* env */, jobject /* obj */) {
    try {
        if (renderer) {
            renderer.reset();
            LOGI("Renderer cleaned up successfully");
        }
    } catch (const std::exception& e) {
        LOGE("Error in cleanup: %s", e.what());
    }
}

} // extern "C"