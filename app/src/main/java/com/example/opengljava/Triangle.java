package com.example.opengljava;

public class Triangle extends Shape {
    private static final String VERTEX_SHADER_PATH = "color_vertex.glsl";
    private static final String FRAGMENT_SHADER_PATH = "color_fragment.glsl";

    private Triangle(Builder builder) {
        super(builder);
    }

    @Override
    public String getVertexShaderPath() {
        return VERTEX_SHADER_PATH;
    }

    @Override
    public String getFragmentShaderPath() {
        return FRAGMENT_SHADER_PATH;
    }

    public static class Builder extends Shape.Builder<Builder> {
        @Override
        protected Builder self() {
            return this;
        }

        @Override
        public Triangle build() {
            // Set default vertices if not provided
            if (super.vertices == null) {
                super.vertices = new float[] {
                        0.0f,  2.0f, 0.0f,     // top          (center, height/2)
                        -2.31f, -2.0f, 0.0f,   // bottom left  (-width/2, -height/2)
                        2.31f, -2.0f, 0.0f     // bottom right (width/2, -height/2)
                };
            }

            // Set default texture coordinates if not provided
            if (super.texCoords == null) {
                super.texCoords = new float[] {
                        0.5f, 1.0f,    // top
                        0.0f, 0.0f,    // bottom left
                        1.0f, 0.0f     // bottom right
                };
            }

            return new Triangle(this);
        }
    }
}