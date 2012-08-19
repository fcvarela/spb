varying vec4 vertex;
varying vec4 vvertex;

uniform sampler2D topoTexture;

void main() {
    gl_TexCoord[0] = gl_MultiTexCoord0;
    
    float radius = 1738140.0;
    
    vec4 heightmap = texture2D(topoTexture, gl_TexCoord[0].st);
    float height = heightmap.a * 65536.0 - 3000.0;
    vertex = vec4(normalize(gl_Vertex.xyz) * (height + radius), 1.0);

    vvertex = gl_ModelViewMatrix * gl_Vertex;
    gl_Position = gl_ModelViewProjectionMatrix * vertex;
}
