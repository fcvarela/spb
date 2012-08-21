varying vec4 vertex;
varying vec4 vvertex;

uniform sampler2D normalTexture;
uniform sampler2D specularTexture;
uniform sampler2D topoTexture;

float lookup( vec2 offSet);
mat3 fromToRotation(vec3 from, vec3 to);

void main() {
    // fetch base color
    vec4 spec = texture2D(specularTexture, gl_TexCoord[0].st);
    vec4 topo = texture2D(topoTexture, gl_TexCoord[0].st);
    
    // fetch normals and rotate
    vec3 normal = texture2D(normalTexture, gl_TexCoord[0].st).xyz;
    normal = (normal * 2.0) - 1.0;
    mat3 rot_matrix = fromToRotation(vec3(0.0, 0.0, 1.0), normalize(vertex.xyz));
    normal = normalize(normal * rot_matrix);
    normal = normalize(gl_NormalMatrix * normal);

    // lighting stuff
    vec4 s = normalize(gl_LightSource[0].position - vertex);
    vec3 light = normalize(s.xyz);

    // finals
    vec4 ambient = vec4(0.0);
    vec4 diffuse = vec4(0.0);
    vec4 specular = vec4(0.0);

    // ambient component
    ambient = gl_LightSource[0].ambient;

    // diffuse component
    float diffuse_coeff = max(0.0, dot(normal, light));
    if (diffuse_coeff > 0.0) {
        diffuse = gl_LightSource[0].diffuse * diffuse_coeff;
        
        // specular component
        vec3 r = -reflect(light, normal);
        vec3 v = normalize(-vvertex.xyz);
        float NdotHV = clamp(dot(r, v), 0.0, 1.0);
        specular = gl_LightSource[0].specular * pow(NdotHV, 8.0) * spec;
    }
    
    float height = topo.a * 16384.0 - (4096.0 + 2048.0);

    vec4 shades[10];
    float heights[10];
    float diffs[10];
    heights[0] = -16384.0;  shades[0] = vec4(  3,  29,  63, 255)/255.0;
    heights[1] = -256.0;    shades[1] = vec4(  3,  29,  63, 255)/255.0;
    heights[2] = -50.0;     shades[2] = vec4(  7, 106, 127, 255)/255.0;
    heights[3] = 0.0;       shades[3] = vec4( 62,  86,  30, 255)/255.0;
    heights[4] = 1024.0;    shades[4] = vec4( 84,  96,  50, 255)/255.0;
    heights[5] = 2048.0;    shades[5] = vec4(130, 127,  97, 255)/255.0;
    heights[6] = 3072.0;    shades[6] = vec4(184, 163, 141, 255)/255.0;
    heights[7] = 4096.0;    shades[7] = vec4(255, 255, 255, 255)/255.0;
    heights[8] = 6144.0;    shades[8] = vec4(128, 255, 255, 255)/255.0;
    heights[9] = 16384.0;   shades[9] = vec4(  0,   0, 255, 255)/255.0;

    vec4 shade;
    float scale = 0.0;
    float coeff = 0.0;

    for (int i=0; i<9; i++) {
        if (height > heights[i] && height <= heights[i+1]) {
            scale = heights[i+1] - heights[i];
            coeff = (height-heights[i]) / scale;
            shade = mix(shades[i], shades[i+1], coeff);
        }
    }

    gl_FragColor = shade * diffuse + specular + (gl_Color + 0.25 * gl_SecondaryColor);
    gl_FragColor.a = 1.0;
}

mat3 fromToRotation(vec3 from, vec3 to) {
    float EPSILON = 0.000001;
    mat3 result;

    vec3 v = cross(from, to);
    float e = dot(from, to);
    float f = (e < 0.0) ? -e : e;
    float hvx, hvz, hvxy, hvxz, hvyz;
        float h = 1.0/(1.0 + e);
        hvx = h * v.x;
        hvz = h * v.z;
        hvxy = hvx * v.y;
        hvxz = hvx * v.z;
        hvyz = hvz * v.y;
        result[0][0] = e + hvx * v.x;
        result[0][1] = hvxy - v.z;
        result[0][2] = hvxz + v.y;

        result[1][0] = hvxy + v.z;
        result[1][1] = e + h * v.y * v.y;
        result[1][2] = hvyz - v.x;

        result[2][0] = hvxz - v.y;
        result[2][1] = hvyz + v.x;
        result[2][2] = e + hvz * v.z;
    return result;
}
