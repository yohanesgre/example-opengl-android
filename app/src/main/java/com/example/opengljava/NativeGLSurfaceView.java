package com.example.opengljava;

import android.content.Context;
import android.opengl.GLSurfaceView;
import android.util.Log;

public class NativeGLSurfaceView extends GLSurfaceView {
    private static final String TAG = "NativeGLSurfaceView";
    private final NativeRenderer nativeRenderer;
    private final NativeGLRenderer renderer;

    public NativeGLSurfaceView(Context context) {
        super(context);

        // Create an OpenGL ES 2.0 context
        setEGLContextClientVersion(2);

        try {
            // Initialize native renderer
            nativeRenderer = new NativeRenderer();
            renderer = new NativeGLRenderer(nativeRenderer);

            // Set the Renderer for drawing on the GLSurfaceView
            setRenderer(renderer);

            // Render continuously
            setRenderMode(GLSurfaceView.RENDERMODE_CONTINUOUSLY);

            Log.i(TAG, "GLSurfaceView setup complete");
        } catch (Exception e) {
            Log.e(TAG, "Error initializing GLSurfaceView", e);
            throw new RuntimeException("Failed to initialize GLSurfaceView", e);
        }
    }

    public void release() {
        try {
            if (renderer != null) {
                renderer.release();
            }
            Log.i(TAG, "GLSurfaceView released");
        } catch (Exception e) {
            Log.e(TAG, "Error releasing GLSurfaceView", e);
        }
    }
}