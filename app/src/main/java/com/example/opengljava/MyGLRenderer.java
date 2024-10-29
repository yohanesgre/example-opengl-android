package com.example.opengljava;

import android.opengl.GLES20;
import android.opengl.GLSurfaceView;
import android.opengl.Matrix;
import android.os.SystemClock;
import android.util.Log;
import java.util.Arrays;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

public class MyGLRenderer implements GLSurfaceView.Renderer {
    private static final String TAG = "MyGLRenderer";

    // vPMatrix is an abbreviation for "Model View Projection Matrix"
    private final float[] vPMatrix = new float[16];
    private final float[] projectionMatrix = new float[16];
    private final float[] viewMatrix = new float[16];
    private float[] rotationMatrix = new float[16];

    private Triangle mTriangle;
    private Square mSquare;

    @Override
    public void onSurfaceCreated(GL10 gl, EGLConfig config) {
        Log.i(TAG, "Surface created - initializing renderer");

        // set background frame color to black
        GLES20.glClearColor(0f, 0f, 0f, 1f);

        try {
            // init triangle
            mTriangle = new Triangle();
            Log.d(TAG, "Triangle initialization successful");

            // init square
            //mSquare = new Square();
            //Log.d(TAG, "Square initialization successful");
        } catch (Exception e) {
            Log.e(TAG, "Error initializing shapes", e);
        }
    }

    @Override
    public void onDrawFrame(GL10 gl) {
        float[] scratch = new float[16];

        // redraw background color
        GLES20.glClear(GLES20.GL_COLOR_BUFFER_BIT);

        try {
            // set the camera position (View matrix)
            Matrix.setLookAtM(viewMatrix, 0, 0, 0, 5, 0f, 0f, 0f, 0.0f, 1.0f, 0.0f);
            Log.v(TAG, "View Matrix: " + matrixToString(viewMatrix));

            // calculate the projection and view transformation
            Matrix.multiplyMM(vPMatrix, 0, projectionMatrix, 0, viewMatrix, 0);
            Log.v(TAG, "Combined VP Matrix: " + matrixToString(vPMatrix));

            // Create a rotation transformation for the triangle
            long time = SystemClock.uptimeMillis() % 4000L;
            float angle = 0.090f * ((int) time);
            Matrix.setRotateM(rotationMatrix, 0, angle, 0, 0, -1.0f);
            Log.v(TAG, "Rotation Matrix (angle=" + angle + "): " + matrixToString(rotationMatrix));

            // Combine the rotation matrix with the projection and camera view
            Matrix.multiplyMM(scratch, 0, vPMatrix, 0, rotationMatrix, 0);
            Log.v(TAG, "Final MVP Matrix: " + matrixToString(scratch));

            mTriangle.draw(scratch);

            if (isGLError()) {
                Log.w(TAG, "GL error detected during frame drawing");
            }
        } catch (Exception e) {
            Log.e(TAG, "Error during frame drawing", e);
        }
    }

    @Override
    public void onSurfaceChanged(GL10 gl, int width, int height) {
        Log.i(TAG, "Surface changed - width: " + width + ", height: " + height);

        GLES20.glViewport(0, 0, width, height);

        float ratio = (float) height / width;
        Log.d(TAG, "Aspect ratio: " + ratio);

        try {
            // this projection matrix is applied to object coordinates
            // in the onDrawFrame() method
            Matrix.frustumM(projectionMatrix, 0, -1f, 1f, -ratio, ratio, 5, 10);
            Log.d(TAG, "Projection Matrix created with frustum:");
            Log.d(TAG, "  Left: -1.0, Right: 1.0");
            Log.d(TAG, "  Bottom: " + (-ratio) + ", Top: " + ratio);
            Log.d(TAG, "  Near: 5.0, Far: 10.0");
            Log.d(TAG, "Projection Matrix: " + matrixToString(projectionMatrix));
        } catch (Exception e) {
            Log.e(TAG, "Error setting up projection matrix", e);
        }
    }

    public static int loadShader(int type, String shaderCode) {
        Log.d(TAG, "Loading shader of type: " + (type == GLES20.GL_VERTEX_SHADER ? "vertex" : "fragment"));

        // create vertex shader type (GLES20.GL_VERTEX_SHADER)
        // or a fragment shader type (GLES20.GL_FRAGMENT_SHADER)
        int shader = GLES20.glCreateShader(type);

        if (shader == 0) {
            Log.e(TAG, "Error creating shader");
            return 0;
        }

        // add the source code to the shader and compile it
        GLES20.glShaderSource(shader, shaderCode);
        GLES20.glCompileShader(shader);

        // check compilation status
        final int[] compileStatus = new int[1];
        GLES20.glGetShaderiv(shader, GLES20.GL_COMPILE_STATUS, compileStatus, 0);

        if (compileStatus[0] == 0) {
            Log.e(TAG, "Shader compilation failed: " + GLES20.glGetShaderInfoLog(shader));
            GLES20.glDeleteShader(shader);
            return 0;
        }

        Log.d(TAG, "Shader compilation successful");
        return shader;
    }

    private boolean isGLError() {
        int error = GLES20.glGetError();
        if (error != GLES20.GL_NO_ERROR) {
            Log.e(TAG, "GL error: 0x" + Integer.toHexString(error));
            return true;
        }
        return false;
    }

    /**
     * Converts a 4x4 matrix to a readable string format
     * @param matrix The 4x4 matrix to convert
     * @return A string representation of the matrix
     */
    private String matrixToString(float[] matrix) {
        StringBuilder sb = new StringBuilder("\n");
        for (int i = 0; i < 4; i++) {
            sb.append("| ");
            for (int j = 0; j < 4; j++) {
                sb.append(String.format("%.4f ", matrix[i * 4 + j]));
            }
            sb.append("|\n");
        }
        return sb.toString();
    }
}