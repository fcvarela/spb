varying vec4 vertex;
varying vec4 vvertex;

uniform sampler2D normalTexture;
uniform sampler2D colorTexture;

uniform sampler2D pnormalTexture;
uniform sampler2D pcolorTexture;

uniform float weight;
uniform float texturesize;
uniform int index;
uniform float far;

mat3 fromToRotation(vec3 from, vec3 to);

void main() {
    const float near = 1000.0;
    const float offset = 0.0;
    gl_FragDepth = (log(near * gl_TexCoord[6].z + offset) / log(near * far + offset));

    vec3 normal = texture2D(normalTexture, gl_TexCoord[0].st).xyz;
    vec4 color = texture2D(colorTexture, gl_TexCoord[0].st);

    // fetch normals and rotate
    normal = (normal * 2.0) - 1.0;
    mat3 rot_matrix = fromToRotation(vec3(0.0, 0.0, 1.0), normalize(vertex.xyz));
    normal = normalize(normal * rot_matrix);
    normal = normalize(gl_NormalMatrix * normal);

    /*
        we may need to morph with our parent for smooth
        transitions. our topology (for texcoord purposes) is
        +---+
        |3|4|
        +-+-+
        |1|2|
        +-+-+
    */
    if (weight > 0.0) {
        vec2 parentCoords;
        vec2 localCoords = vec2(gl_TexCoord[0].s, gl_TexCoord[0].t);
        if (index == 3)
            parentCoords = 1.0/texturesize+vec2(localCoords.s/2.0, localCoords.t/2.0);
        if (index == 4)
            parentCoords = vec2(0.5-1.0/texturesize+localCoords.s/2.0, 1.0/texturesize+localCoords.t/2.0);
        if (index == 1)
            parentCoords = vec2(1.0/texturesize+localCoords.s/2.0, 0.5-1.0/texturesize+localCoords.t/2.0);
        if (index == 2)
            parentCoords = 0.5-1.0/texturesize+vec2(localCoords.s/2.0, localCoords.t/2.0);
        
        vec3 pnormal = texture2D(pnormalTexture, parentCoords).xyz;
        vec4 pcolor = texture2D(pcolorTexture, parentCoords);
        color = mix(color, pcolor, weight);
        
        pnormal = (pnormal * 2.0) - 1.0;
        pnormal = normalize(pnormal * rot_matrix);
        pnormal = normalize(gl_NormalMatrix * pnormal);

        normal = mix(normal, pnormal, weight);
    }

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
        specular = gl_LightSource[0].specular * pow(max(dot(r, v), 0.0), 8.0) * color.a;
    }

    gl_FragColor = color * (ambient + diffuse) + specular + (gl_Color + 0.25 * gl_SecondaryColor);
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
