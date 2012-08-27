varying vec3 v3Pos;

void main(void) {
    vec4 vertex = vec4(gl_Vertex.xyz * 1738140.0, 1.0);
	gl_Position = gl_ModelViewProjectionMatrix * vertex;
    v3Pos = gl_Vertex.xyz;
}
