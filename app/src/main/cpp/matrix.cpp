// matrix.cpp
#include "include/matrix.h"
#include <cstring>
#include <android/log.h>

#define LOG_TAG "MatrixHelper"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

void Matrix::logMatrix(const char* label, const float* m, int offset) {
    LOGI("Matrix %s:", label);
    for (int i = 0; i < 4; i++) {
        LOGI("Row %d: %.6f, %.6f, %.6f, %.6f",
             i, m[offset + i*4], m[offset + i*4 + 1],
             m[offset + i*4 + 2], m[offset + i*4 + 3]);
    }
}

void Matrix::setIdentityM(float* m, int offset) {
    for (int i = 0; i < 16; i++) {
        m[offset + i] = 0.0f;
    }
    m[offset + 0] = 1.0f;   // M11
    m[offset + 5] = 1.0f;   // M22
    m[offset + 10] = 1.0f;  // M33
    m[offset + 15] = 1.0f;  // M44

    logMatrix("Identity", m, offset);
}

void Matrix::multiplyMM(float* result, int resultOffset,
                        const float* lhs, int lhsOffset,
                        const float* rhs, int rhsOffset) {
    logMatrix("Left Matrix", lhs, lhsOffset);
    logMatrix("Right Matrix", rhs, rhsOffset);

    float temp[16];
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            float sum = 0.0f;
            for (int k = 0; k < 4; k++) {
                int lhsIndex = lhsOffset + (i * 4 + k);
                int rhsIndex = rhsOffset + (k * 4 + j);
                float lhsValue = lhs[lhsIndex];
                float rhsValue = rhs[rhsIndex];
                sum += lhsValue * rhsValue;
                LOGI("multiply [%d,%d]: %.6f * %.6f = %.6f (sum = %.6f)",
                     i, j, lhsValue, rhsValue, lhsValue * rhsValue, sum);
            }
            temp[i * 4 + j] = sum;
        }
    }

    memcpy(result + resultOffset, temp, 16 * sizeof(float));
    logMatrix("Multiplication Result", result, resultOffset);
}

void Matrix::frustumM(float* m, int offset,
                      float left, float right,
                      float bottom, float top,
                      float near, float far) {
    LOGI("Frustum: L=%.6f, R=%.6f, B=%.6f, T=%.6f, N=%.6f, F=%.6f",
         left, right, bottom, top, near, far);

    float r_width = 1.0f / (right - left);
    float r_height = 1.0f / (top - bottom);
    float r_depth = 1.0f / (near - far);
    float x = 2.0f * near * r_width;
    float y = 2.0f * near * r_height;
    float A = (right + left) * r_width;
    float B = (top + bottom) * r_height;
    float C = (far + near) * r_depth;
    float D = 2.0f * far * near * r_depth;

    m[offset + 0] = x;
    m[offset + 1] = 0.0f;
    m[offset + 2] = 0.0f;
    m[offset + 3] = 0.0f;

    m[offset + 4] = 0.0f;
    m[offset + 5] = y;
    m[offset + 6] = 0.0f;
    m[offset + 7] = 0.0f;

    m[offset + 8] = A;
    m[offset + 9] = B;
    m[offset + 10] = C;
    m[offset + 11] = -1.0f;

    m[offset + 12] = 0.0f;
    m[offset + 13] = 0.0f;
    m[offset + 14] = D;
    m[offset + 15] = 0.0f;

    logMatrix("Frustum", m, offset);
}

void Matrix::setLookAtM(float* m, int offset,
                        float eyeX, float eyeY, float eyeZ,
                        float centerX, float centerY, float centerZ,
                        float upX, float upY, float upZ) {
    LOGI("LookAt: Eye(%.6f,%.6f,%.6f), Center(%.6f,%.6f,%.6f), Up(%.6f,%.6f,%.6f)",
         eyeX, eyeY, eyeZ, centerX, centerY, centerZ, upX, upY, upZ);

    // Compute forward vector (z-axis)
    float fx = centerX - eyeX;
    float fy = centerY - eyeY;
    float fz = centerZ - eyeZ;

    // Normalize forward vector
    float len = std::sqrt(fx * fx + fy * fy + fz * fz);
    if (len != 0.0f) {
        fx /= len;
        fy /= len;
        fz /= len;
    }

    // Compute right vector (x-axis) as cross product of forward and up
    float sx = fy * upZ - fz * upY;
    float sy = fz * upX - fx * upZ;
    float sz = fx * upY - fy * upX;

    // Normalize right vector
    len = std::sqrt(sx * sx + sy * sy + sz * sz);
    if (len != 0.0f) {
        sx /= len;
        sy /= len;
        sz /= len;
    }

    // Compute up vector (y-axis) as cross product of right and forward
    float ux = sy * fz - sz * fy;
    float uy = sz * fx - sx * fz;
    float uz = sx * fy - sy * fx;

    // Build the rotation matrix
    m[offset + 0] = sx;
    m[offset + 1] = ux;
    m[offset + 2] = -fx;
    m[offset + 3] = 0.0f;

    m[offset + 4] = sy;
    m[offset + 5] = uy;
    m[offset + 6] = -fy;
    m[offset + 7] = 0.0f;

    m[offset + 8] = sz;
    m[offset + 9] = uz;
    m[offset + 10] = -fz;
    m[offset + 11] = 0.0f;

    m[offset + 12] = 0.0f;
    m[offset + 13] = 0.0f;
    m[offset + 14] = 0.0f;
    m[offset + 15] = 1.0f;

    logMatrix("View Matrix Before Translation", m, offset);

    // Apply translation
    translateM(m, offset, -eyeX, -eyeY, -eyeZ);

    logMatrix("Final View Matrix", m, offset);
}

void Matrix::translateM(float* m, int offset,
                        float x, float y, float z) {
    LOGI("Translation: (%.6f, %.6f, %.6f)", x, y, z);
    logMatrix("Before Translation", m, offset);

    for (int i = 0; i < 4; i++) {
        int mi = offset + i;
        m[12 + mi] += m[mi] * x + m[4 + mi] * y + m[8 + mi] * z;
    }

    logMatrix("After Translation", m, offset);
}

void Matrix::scaleM(float* m, int offset,
                    float x, float y, float z) {
    LOGI("Scale: (%.6f, %.6f, %.6f)", x, y, z);
    logMatrix("Before Scale", m, offset);

    for (int i = 0; i < 4; i++) {
        m[offset + i] *= x;
        m[offset + 4 + i] *= y;
        m[offset + 8 + i] *= z;
    }

    logMatrix("After Scale", m, offset);
}

void Matrix::rotateM(float* m, int offset,
                     float angle, float x, float y, float z) {
    LOGI("Rotation: %.6f degrees around (%.6f,%.6f,%.6f)", angle, x, y, z);
    logMatrix("Before Rotation", m, offset);

    float rad = angle * static_cast<float>(M_PI / 180.0f);
    float s = std::sin(rad);
    float c = std::cos(rad);

    float len = std::sqrt(x * x + y * y + z * z);
    if (len != 1.0f) {
        float recipLen = 1.0f / len;
        x *= recipLen;
        y *= recipLen;
        z *= recipLen;
    }

    float nc = 1.0f - c;
    float xy = x * y;
    float yz = y * z;
    float zx = z * x;
    float xs = x * s;
    float ys = y * s;
    float zs = z * s;

    float r[16] = {
            x*x*nc + c,  xy*nc + zs,  zx*nc - ys,  0.0f,
            xy*nc - zs,  y*y*nc + c,  yz*nc + xs,  0.0f,
            zx*nc + ys,  yz*nc - xs,  z*z*nc + c,  0.0f,
            0.0f,        0.0f,        0.0f,        1.0f
    };

    float temp[16];
    multiplyMM(temp, 0, m, offset, r, 0);
    memcpy(m + offset, temp, 16 * sizeof(float));

    logMatrix("After Rotation", m, offset);
}