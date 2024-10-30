package com.example.opengljava;

import android.content.Context;
import android.content.res.AssetManager;
import android.util.Log;

public class NativeRenderer {
    private static final String TAG = "NativeRenderer";
    private boolean isInitialized = false;

    static {
        try {
            System.loadLibrary("native-renderer");
            Log.i(TAG, "Native library loaded successfully");
        } catch (UnsatisfiedLinkError e) {
            Log.e(TAG, "Failed to load native library", e);
            throw e;
        }
    }

    public native void init(AssetManager assetManager);
    public native void setupView(int width, int height);
    public native void cleanup();
    public native void drawShape(
            float[] vertices,
            float[] texCoords,
            float[] modelMatrix,
            String vertexShader,
            String fragmentShader,
            float scaleX, float scaleY, float scaleZ,
            float rotationAngle,
            float rotationX, float rotationY, float rotationZ
    );

    public void initialize(AssetManager assetManager) {
        try {
            init(assetManager);
            isInitialized = true;
            Log.i(TAG, "Renderer initialized successfully");
        } catch (Exception e) {
            Log.e(TAG, "Failed to initialize renderer", e);
            throw new RuntimeException("Failed to initialize renderer", e);
        }
    }

    public void render(Shape shape) {
        if (!isInitialized) {
            Log.e(TAG, "Renderer not initialized");
            throw new IllegalStateException("Renderer must be initialized before rendering");
        }

        try {
            float[] vertices = shape.getVertices();
            float[] texCoords = shape.getTexCoords();
            float[] modelMatrix = shape.getModelMatrix();
            String vertexShader = shape.getVertexShaderPath();
            String fragmentShader = shape.getFragmentShaderPath();

            // Get transformation parameters
            float[] scale = shape.getScale();
            float rotationAngle = shape.getRotationAngle();
            float[] rotationAxis = shape.getRotationAxis();

            // Validate inputs
            if (vertices == null || vertices.length == 0) {
                throw new IllegalArgumentException("Invalid vertices data");
            }
            if (texCoords == null || texCoords.length == 0) {
                throw new IllegalArgumentException("Invalid texture coordinates data");
            }
            if (modelMatrix == null || modelMatrix.length != 16) {
                throw new IllegalArgumentException("Invalid model matrix");
            }
            if (vertexShader == null || vertexShader.isEmpty()) {
                throw new IllegalArgumentException("Invalid vertex shader path");
            }
            if (fragmentShader == null || fragmentShader.isEmpty()) {
                throw new IllegalArgumentException("Invalid fragment shader path");
            }
            if (scale == null || scale.length != 3) {
                throw new IllegalArgumentException("Invalid scale values");
            }
            if (rotationAxis == null || rotationAxis.length != 3) {
                throw new IllegalArgumentException("Invalid rotation axis");
            }

            // Log transformation parameters for debugging
            Log.d(TAG, String.format("Rendering shape with scale (%.2f, %.2f, %.2f)",
                    scale[0], scale[1], scale[2]));
            Log.d(TAG, String.format("Rotation: %.2f degrees around axis (%.2f, %.2f, %.2f)",
                    rotationAngle, rotationAxis[0], rotationAxis[1], rotationAxis[2]));

            drawShape(
                    vertices,
                    texCoords,
                    modelMatrix,
                    vertexShader,
                    fragmentShader,
                    scale[0], scale[1], scale[2],
                    rotationAngle,
                    rotationAxis[0], rotationAxis[1], rotationAxis[2]
            );
        } catch (Exception e) {
            Log.e(TAG, "Error rendering shape", e);
            throw new RuntimeException("Failed to render shape", e);
        }
    }

    public void updateView(int width, int height) {
        if (!isInitialized) {
            Log.e(TAG, "Renderer not initialized");
            throw new IllegalStateException("Renderer must be initialized before updating view");
        }

        if (width <= 0 || height <= 0) {
            throw new IllegalArgumentException(
                    String.format("Invalid dimensions: %dx%d", width, height));
        }

        try {
            setupView(width, height);
            Log.i(TAG, String.format("View updated: %dx%d", width, height));
        } catch (Exception e) {
            Log.e(TAG, "Failed to update view", e);
            throw new RuntimeException("Failed to update view", e);
        }
    }

    public void release() {
        try {
            cleanup();
            isInitialized = false;
            Log.i(TAG, "Renderer released successfully");
        } catch (Exception e) {
            Log.e(TAG, "Error releasing renderer", e);
            throw new RuntimeException("Failed to release renderer", e);
        }
    }

    @Override
    protected void finalize() throws Throwable {
        try {
            if (isInitialized) {
                Log.w(TAG, "Renderer was not properly released before finalization");
                release();
            }
        } finally {
            super.finalize();
        }
    }
}