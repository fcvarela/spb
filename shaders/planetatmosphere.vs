varying vec3 v3Pos;

void main(void) {
    v3Pos = gl_Vertex.xyz / 1738140.0;
    gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
}
