varying vec3 vertex_light_position;
varying vec4 orig_vertex;
uniform sampler2D colorTexture;
uniform sampler2D normalTexture;

mat3 fromToRotation(vec3 from, vec3 to);

void main() {
    vec3 normal = texture2D(normalTexture, gl_TexCoord[0].st).xyz;
    normal = (normal * 2.0) - 1.0;

    mat3 rot_matrix = fromToRotation(vec3(0.0, 0.0, 1.0), normalize(orig_vertex.xyz));
    
    normal = normalize(normal * rot_matrix);
    normal = normalize(gl_NormalMatrix * normal);

    float diffuse_value = max(dot(normal, vertex_light_position), 0.0);
    gl_FragColor = texture2D(colorTexture, gl_TexCoord[0].xy) * diffuse_value;
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
