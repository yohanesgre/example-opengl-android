package com.example.opengljava;

import android.content.res.AssetManager;
import android.opengl.GLSurfaceView;
import android.opengl.Matrix;
import android.os.SystemClock;
import android.util.Log;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

public class NativeGLRenderer implements GLSurfaceView.Renderer {
    private static final String TAG = "NativeGLRenderer";
    private final NativeRenderer nativeRenderer;
    private final Triangle triangle;
    private boolean isInitialized = false;
    private final AssetManager assetManager;

    public NativeGLRenderer(NativeRenderer nativeRenderer, AssetManager assetManager) {
        if (nativeRenderer == null) {
            throw new IllegalArgumentException("NativeRenderer cannot be null");
        }
        this.nativeRenderer = nativeRenderer;

        if (assetManager == null) {
            throw new IllegalArgumentException("AssetManager cannot be null");
        }
        this.assetManager = assetManager;

        this.triangle = new Triangle.Builder().scale(1f, 1f, 1f).build();
        Log.i(TAG, "NativeGLRenderer created");
    }

    @Override
    public void onSurfaceCreated(GL10 gl, EGLConfig config) {
        try {
            nativeRenderer.initialize(assetManager);
            isInitialized = true;
            Log.i(TAG, "Surface created and renderer initialized");
        } catch (Exception e) {
            Log.e(TAG, "Error initializing renderer", e);
            isInitialized = false;
        }
    }

    @Override
    public void onSurfaceChanged(GL10 gl, int width, int height) {
        try {
            if (!isInitialized) {
                Log.w(TAG, "Renderer not initialized, attempting to initialize");
                nativeRenderer.initialize(assetManager);
                isInitialized = true;
            }
            nativeRenderer.updateView(width, height);
            Log.i(TAG, String.format("Surface changed: %dx%d", width, height));
        } catch (Exception e) {
            Log.e(TAG, "Error updating view", e);
            isInitialized = false;
        }
    }

    @Override
    public void onDrawFrame(GL10 gl) {
        if (!isInitialized) {
            Log.e(TAG, "Cannot draw frame: renderer not initialized");
            return;
        }

        try {
            // Update rotation
            long time = SystemClock.uptimeMillis() % 4000L;
            float angle = 0.090f * ((int) time);

            triangle.setRotationZ(angle);

            // Draw using native renderer
            nativeRenderer.render(triangle);
        } catch (Exception e) {
            Log.e(TAG, "Error drawing frame", e);
            isInitialized = false;
        }
    }

    public void release() {
        try {
            if (isInitialized) {
                nativeRenderer.release();
                isInitialized = false;
                Log.i(TAG, "Renderer released");
            }
        } catch (Exception e) {
            Log.e(TAG, "Error releasing renderer", e);
        }
    }
}