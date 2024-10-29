package com.example.opengljava;

public abstract class Shape {
    protected float[] vertices;
    protected float[] texCoords;
    protected float[] modelMatrix;
    protected float[] scale;
    protected float rotationAngle;
    protected float[] rotationAxis;

    protected Shape(Builder<?> builder) {
        this.vertices = builder.vertices;
        this.texCoords = builder.texCoords;
        this.modelMatrix = builder.modelMatrix;
        this.scale = builder.scale;
        this.rotationAngle = builder.rotationAngle;
        this.rotationAxis = builder.rotationAxis;
    }

    // Existing getters
    public float[] getVertices() { return vertices; }
    public float[] getTexCoords() { return texCoords; }
    public float[] getModelMatrix() { return modelMatrix; }
    public float[] getScale() { return scale; }
    public float getRotationAngle() { return rotationAngle; }
    public float[] getRotationAxis() { return rotationAxis; }
    public abstract String getVertexShaderPath();
    public abstract String getFragmentShaderPath();

    // Direct scaling methods
    public void setScale(float x, float y, float z) {
        this.scale = new float[]{x, y, z};
    }

    public void setUniformScale(float scale) {
        setScale(scale, scale, scale);
    }

    public void scaleBy(float x, float y, float z) {
        this.scale = new float[]{
                this.scale[0] * x,
                this.scale[1] * y,
                this.scale[2] * z
        };
    }

    public void scaleUniformly(float factor) {
        scaleBy(factor, factor, factor);
    }

    // Direct rotation methods
    public void setRotation(float angle, float x, float y, float z) {
        this.rotationAngle = angle;
        this.rotationAxis = new float[]{x, y, z};
    }

    public void setRotationX(float angle) {
        setRotation(angle, 1.0f, 0.0f, 0.0f);
    }

    public void setRotationY(float angle) {
        setRotation(angle, 0.0f, 1.0f, 0.0f);
    }

    public void setRotationZ(float angle) {
        setRotation(angle, 0.0f, 0.0f, 1.0f);
    }

    public void rotate(float angle) {
        this.rotationAngle = (this.rotationAngle + angle) % 360.0f;
    }

    public void rotateAround(float angle, float x, float y, float z) {
        // Normalize the rotation axis
        float length = (float) Math.sqrt(x * x + y * y + z * z);
        if (length > 0.0001f) {
            x /= length;
            y /= length;
            z /= length;
        }
        this.rotationAngle = (this.rotationAngle + angle) % 360.0f;
        this.rotationAxis = new float[]{x, y, z};
    }

    // Builder class (unchanged)
    public static abstract class Builder<T extends Builder<T>> {
        float[] vertices;
        float[] texCoords;
        private float[] modelMatrix;
        private float[] scale = {1.0f, 1.0f, 1.0f};  // Default: no scaling
        private float rotationAngle = 0.0f;           // Default: no rotation
        private float[] rotationAxis = {0.0f, 1.0f, 0.0f};  // Default: Y-axis

        public Builder() {
            // Initialize model matrix as identity matrix
            modelMatrix = new float[16];
            modelMatrix[0] = 1.0f;
            modelMatrix[5] = 1.0f;
            modelMatrix[10] = 1.0f;
            modelMatrix[15] = 1.0f;
        }

        public T vertices(float[] vertices) {
            this.vertices = vertices;
            return self();
        }

        public T texCoords(float[] texCoords) {
            this.texCoords = texCoords;
            return self();
        }

        public T modelMatrix(float[] modelMatrix) {
            if (modelMatrix != null && modelMatrix.length == 16) {
                this.modelMatrix = modelMatrix;
            }
            return self();
        }

        public T scale(float x, float y, float z) {
            this.scale = new float[]{x, y, z};
            return self();
        }

        public T uniformScale(float scale) {
            return scale(scale, scale, scale);
        }

        public T rotation(float angle, float x, float y, float z) {
            this.rotationAngle = angle;
            this.rotationAxis = new float[]{x, y, z};
            return self();
        }

        public T rotateX(float angle) {
            return rotation(angle, 1.0f, 0.0f, 0.0f);
        }

        public T rotateY(float angle) {
            return rotation(angle, 0.0f, 1.0f, 0.0f);
        }

        public T rotateZ(float angle) {
            return rotation(angle, 0.0f, 0.0f, 1.0f);
        }

        protected abstract T self();
        public abstract Shape build();
    }
}