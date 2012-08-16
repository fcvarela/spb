varying vec3 vertex_light_position;
varying vec4 orig_vertex;

uniform sampler2D topoTexture;

void main()
{
    gl_TexCoord[0] = gl_MultiTexCoord0;
    
    float radius = 1.0;
    
    vec4 heightmap = texture2D(topoTexture, gl_TexCoord[0].st);
    float height = heightmap.g * 255.0 + heightmap.r;
    height /= 173814.0;
    orig_vertex = vec4(normalize(gl_Vertex.xyz) * (height + radius), 1.0);
    
    vec4 v_vertex = gl_ModelViewMatrix * orig_vertex;
    gl_Position = gl_ModelViewProjectionMatrix * orig_vertex;

    vertex_light_position = normalize(gl_LightSource[0].position.xyz);
}
