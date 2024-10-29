package com.example.opengljava;

import android.os.Bundle;
import android.util.Log;

import androidx.appcompat.app.AppCompatActivity;

public class MainActivity extends AppCompatActivity {
    private static final String TAG = "MainActivity";
    private NativeGLSurfaceView glSurfaceView;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        try {
            glSurfaceView = new NativeGLSurfaceView(this);
            setContentView(glSurfaceView);
        } catch (Exception e) {
            Log.e(TAG, "Error creating GLSurfaceView", e);
            finish();
        }
    }

    @Override
    protected void onPause() {
        super.onPause();
        if (glSurfaceView != null) {
            glSurfaceView.onPause();
        }
    }

    @Override
    protected void onResume() {
        super.onResume();
        if (glSurfaceView != null) {
            glSurfaceView.onResume();
        }
    }

    @Override
    protected void onDestroy() {
        if (glSurfaceView != null) {
            glSurfaceView.release();
        }
        super.onDestroy();
    }
}