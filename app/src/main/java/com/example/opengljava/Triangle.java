package com.example.opengljava;

public class Triangle extends Shape {
    public Triangle() {
        vertices = new float[] {
                0.0f,  0.5f, 0.0f,
                -0.2887f, 0f, 0.0f,
                0.2887f,  0f, 0.0f
        };

        texCoords = new float[] {
                0.0f, 0.0f,
                1.0f, 0.0f,
                0.0f, 1.0f
        };

        modelMatrix = new float[16];
        // Initialize model matrix as identity
    }

    @Override
    public String getVertexShaderPath() {
        return "shaders/color_vertex.glsl";
    }

    @Override
    public String getFragmentShaderPath() {
        return "shaders/color_fragment.glsl";
    }
}