package com.example.opengljava;


public abstract class Shape {
    protected float[] vertices;
    protected float[] texCoords;
    protected float[] modelMatrix;

    public Shape() {
        modelMatrix = new float[16];
        // Initialize as identity matrix
        modelMatrix[0] = 1.0f;
        modelMatrix[5] = 1.0f;
        modelMatrix[10] = 1.0f;
        modelMatrix[15] = 1.0f;
    }

    public float[] getVertices() { return vertices; }
    public float[] getTexCoords() { return texCoords; }
    public float[] getModelMatrix() { return modelMatrix; }

    public void setModelMatrix(float[] matrix) {
        System.arraycopy(matrix, 0, modelMatrix, 0, 16);
    }

    public abstract String getVertexShaderPath();
    public abstract String getFragmentShaderPath();
}