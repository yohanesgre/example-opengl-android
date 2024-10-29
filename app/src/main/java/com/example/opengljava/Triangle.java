package com.example.opengljava;

import android.opengl.GLES20;
import android.opengl.Matrix;
import android.util.Log;

import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.nio.FloatBuffer;

public class Triangle {
    private static final String TAG = "Triangle"; // Tag for logging

    private final String vertexShaderCode =
            "uniform mat4 uMVPMatrix;" +
                    "attribute vec4 vPosition;" +
                    "attribute vec2 texCoord;" +
                    "varying vec2 uv;" +
                    "void main() {" +
                    "    gl_Position = uMVPMatrix * vPosition;" +
                    "    uv = texCoord;" +
                    "}";

    private final String fragmentShaderCode =
            "precision mediump float;" +
                    "varying vec2 uv;" +
                    "void main() {" +
                    "    // Calculate barycentric coordinates (u, v, w) using uv\n" +
                    "    vec3 barycentric = vec3(1.0 - uv.x - uv.y, uv.x, uv.y);" +
                    "    // Assign RGB values to the triangle vertices\n" +
                    "    vec3 vertexColors[3];" +
                    "    vertexColors[0] = vec3(1.0, 0.0, 0.0);" + // Red
                    "    vertexColors[1] = vec3(0.0, 1.0, 0.0);" + // Green
                    "    vertexColors[2] = vec3(0.0, 0.0, 1.0);" + // Blue
                    "    // Interpolate colors using barycentric coordinates\n" +
                    "    vec3 color = barycentric.x * vertexColors[0] +" +
                    "                 barycentric.y * vertexColors[1] +" +
                    "                 barycentric.z * vertexColors[2];" +
                    "    gl_FragColor = vec4(color, 1.0);" +
                    "}";

    private FloatBuffer vertexBuffer;
    private FloatBuffer texCoordBuffer;

    static final int COORDS_PER_VERTEX = 3;
    static float[] triangleCords = {
            0.0f, 0.5f, 0.0f,      // top
            -0.2887f, 0f, 0.0f,    // bottom left
            0.2887f, 0f, 0.0f      // bottom right
    };

    static float[] uvCords = {
            0.0f, 0.0f,    // top
            1.0f, 0.0f,    // bottom left
            0.0f, 1.0f     // bottom right
    };

    private final int mProgram;

    // Helper method to print matrix
    private void printMatrix(String label, float[] matrix) {
        StringBuilder sb = new StringBuilder(label + ":\n");
        for (int i = 0; i < 4; i++) {
            sb.append("[ ");
            for (int j = 0; j < 4; j++) {
                sb.append(String.format("%.4f ", matrix[i * 4 + j]));
            }
            sb.append("]\n");
        }
        Log.d(TAG, sb.toString());
    }

    public Triangle() {
        // Initialize vertex byte buffer for shape coordinates
        ByteBuffer vertex_bb = ByteBuffer.allocateDirect(triangleCords.length * 4);
        vertex_bb.order(ByteOrder.nativeOrder());
        vertexBuffer = vertex_bb.asFloatBuffer();
        vertexBuffer.put(triangleCords);
        vertexBuffer.position(0);

        // Initialize texture coordinate buffer
        ByteBuffer tex_bb = ByteBuffer.allocateDirect(uvCords.length * 4);
        tex_bb.order(ByteOrder.nativeOrder());
        texCoordBuffer = tex_bb.asFloatBuffer();
        texCoordBuffer.put(uvCords);
        texCoordBuffer.position(0);

        int vertexShader = MyGLRenderer.loadShader(GLES20.GL_VERTEX_SHADER, vertexShaderCode);
        int fragmentShader = MyGLRenderer.loadShader(GLES20.GL_FRAGMENT_SHADER, fragmentShaderCode);

        mProgram = GLES20.glCreateProgram();
        GLES20.glAttachShader(mProgram, vertexShader);
        GLES20.glAttachShader(mProgram, fragmentShader);
        GLES20.glLinkProgram(mProgram);
    }

    private int positionHandle;
    private int texCoordHandle;
    private int mvpMatrixHandle;

    private final int vertexCount = triangleCords.length / COORDS_PER_VERTEX;
    private final int vertexStride = COORDS_PER_VERTEX * 4;

    public void draw(float[] mvpMatrix) {
        // Log the input MVP matrix
        printMatrix("Input MVP Matrix", mvpMatrix);

        GLES20.glUseProgram(mProgram);

        positionHandle = GLES20.glGetAttribLocation(mProgram, "vPosition");
        GLES20.glEnableVertexAttribArray(positionHandle);
        GLES20.glVertexAttribPointer(positionHandle, COORDS_PER_VERTEX,
                GLES20.GL_FLOAT, false, vertexStride, vertexBuffer);

        texCoordHandle = GLES20.glGetAttribLocation(mProgram, "texCoord");
        GLES20.glEnableVertexAttribArray(texCoordHandle);
        GLES20.glVertexAttribPointer(texCoordHandle, 2,
                GLES20.GL_FLOAT, false, 0, texCoordBuffer);

        mvpMatrixHandle = GLES20.glGetUniformLocation(mProgram, "uMVPMatrix");

        Matrix.scaleM(mvpMatrix, 0, 2f, 2f, 0f);
        // Log the scaled matrix
        printMatrix("Final Scaled Matrix", mvpMatrix);

        // Apply scaling and translation
        Matrix.translateM(mvpMatrix, 0, 0f, -0.15f, 0f);
        // Log the translated matrix
        printMatrix("Final Translated Matrix", mvpMatrix);

        // Apply the projection and view transformation
        GLES20.glUniformMatrix4fv(mvpMatrixHandle, 1, false, mvpMatrix, 0);

        // Draw the triangle
        GLES20.glDrawArrays(GLES20.GL_TRIANGLES, 0, vertexCount);

        // Disable vertex arrays
        GLES20.glDisableVertexAttribArray(positionHandle);
        GLES20.glDisableVertexAttribArray(texCoordHandle);
    }
}