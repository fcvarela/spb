varying vec4 vertex;

void main() {
}

void venus() {
    float Ka = 1;
    float Kd = 1;
    float offset = 1;
    float scale = 0.6;
    float twist = 0.22;
    float H = 0.65;
    float octaves = 8;

    vec2 Ptexture = gl_TexCoords[0].st;
    vec3 PP = vertex.xyz;

    float rsq;
    float angle;
    float value;
    float filtwidth;

    vec2 PtN = gl_TexCoord[0].st
    rsq = PtN.x*PtN.x + PtN.y*PtNy;
    angle = twist * (2*3.1428) * rsq;

    // rotate coords
    PP = rotate(Ptexture, angle, vec3(0.0, 0.0, 0.0), vec3(0.0, 0.0, 1.0));
    
    /* Compute Coriolis-distorted clouds */
    value = abs(offset + scale * fBm(PP, filtwidth, octaves,2, H));
    
    /* Shade like matte, but with color scaled by cloud color */
    /* Oi = output opacity */
    /* Os = input surface opacity */
    /* Ci = output color */
    /* Cs = input source color */

    Ci = Oi * (value * Cs) * (Ka * ambient() + Kd * diffuse(faceforward(normalize(N),1)));
}

vec2 rotate(vec2 vector, float angle, vec3 point, vec3(axis)) {
    return vector;
}