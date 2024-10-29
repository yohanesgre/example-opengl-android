// matrix.h
#ifndef OPENGLJAVA_MATRIX_H
#define OPENGLJAVA_MATRIX_H

#pragma once
#include <cmath>

class Matrix {
public:
    // Core matrix operations
    static void setIdentityM(float* m, int offset);
    static void multiplyMM(float* result, int resultOffset,
                           const float* lhs, int lhsOffset,
                           const float* rhs, int rhsOffset);

    // Camera and projection matrices
    static void frustumM(float* m, int offset,
                         float left, float right,
                         float bottom, float top,
                         float near, float far);

    static void setLookAtM(float* m, int offset,
                           float eyeX, float eyeY, float eyeZ,
                           float centerX, float centerY, float centerZ,
                           float upX, float upY, float upZ);

    // Transformation matrices
    static void translateM(float* m, int offset,
                           float x, float y, float z);
    static void scaleM(float* m, int offset,
                       float x, float y, float z);
    static void rotateM(float* m, int offset,
                        float angle, float x, float y, float z);

private:
    static void logMatrix(const char* label, const float* m, int offset);
};

#endif //OPENGLJAVA_MATRIX_H