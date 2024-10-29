precision mediump float;
varying vec2 uv;

void main() {
    // Calculate barycentric coordinates
    vec3 barycentric = vec3(1.0 - uv.x - uv.y, uv.x, uv.y);

    // Vertex colors
    vec3 vertexColors[3];
    vertexColors[0] = vec3(1.0, 0.0, 0.0);  // Red
    vertexColors[1] = vec3(0.0, 1.0, 0.0);  // Green
    vertexColors[2] = vec3(0.0, 0.0, 1.0);  // Blue

    // Interpolate colors
    vec3 color = barycentric.x * vertexColors[0] +
                 barycentric.y * vertexColors[1] +
                 barycentric.z * vertexColors[2];

    gl_FragColor = vec4(color, 1.0);
}