// jni_bridge.cpp
#include <jni.h>
#include "include/gl_renderer.h"
#include <android/log.h>
#include <stdexcept>
#include <memory>

#define LOG_TAG "JNIBridge"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

// Global renderer instance using smart pointer for automatic cleanup
static std::unique_ptr<Renderer> renderer;

// Helper function to check for GL errors
static void checkGLError(const char* operation) {
    GLenum error;
    while ((error = glGetError()) != GL_NO_ERROR) {
        LOGE("GL Error after %s: 0x%x", operation, error);
    }
}

// RAII wrapper for JNI float arrays
class JFloatArrayGuard {
public:
    JFloatArrayGuard(JNIEnv* env, jfloatArray array)
            : env(env), array(array), ptr(nullptr) {
        if (array) {
            ptr = env->GetFloatArrayElements(array, nullptr);
        }
    }

    ~JFloatArrayGuard() {
        if (ptr && array) {
            env->ReleaseFloatArrayElements(array, ptr, JNI_ABORT);
        }
    }

    jfloat* get() { return ptr; }
    operator bool() const { return ptr != nullptr; }

private:
    JNIEnv* env;
    jfloatArray array;
    jfloat* ptr;
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
        jstring vertexShader, jstring fragmentShader) {

    if (!renderer) {
        LOGE("Renderer not initialized");
        return;
    }

    try {
        // Get vertex count and validate
        jsize vertexCount = env->GetArrayLength(vertices) / 3;
        if (vertexCount <= 0) {
            throw std::runtime_error("Invalid vertex count");
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
                vertexCount,
                texCoordPtr.get(),
                modelMatrixPtr.get()
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