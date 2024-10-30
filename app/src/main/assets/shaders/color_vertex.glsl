uniform mat4 uMVPMatrix;
attribute vec4 vPosition;
attribute vec2 texCoord;
varying vec2 uv;

void main() {
    gl_Position = uMVPMatrix * vPosition;
    uv = texCoord;
}