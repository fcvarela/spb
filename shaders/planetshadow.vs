varying vec4 vertex;
varying vec4 vvertex;
uniform sampler2D topoTexture;

void main()
{
	float radius = 17381400.0;
    
    vec4 heightmap = texture2D(topoTexture, gl_TexCoord[0].st);
    float height = gl_Vertex.x/2.0 + gl_Vertex.y/2.0;//heightmap.a * 32768.0 - 3000.0;
    vertex = vec4(normalize(gl_Vertex.xyz) * (height + radius), 1.0);
    vvertex = gl_ModelViewMatrix * vertex;
    gl_Position = gl_ModelViewProjectionMatrix * vertex;
}