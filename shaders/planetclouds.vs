varying vec4 vertex;

void main(void) {
    gl_TexCoord[0] = gl_MultiTexCoord0;

    vec4 vertex = vec4(gl_Vertex.xyz * 1738140.0*1.02, 1.0);
    gl_Position = gl_ModelViewProjectionMatrix * vertex;
}
