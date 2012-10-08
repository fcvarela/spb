#ifdef _VERTEX_

uniform float weight;
uniform int index;
uniform float texturesize;

varying vec4 vertex;
varying vec4 vvertex;

uniform sampler2D topoTexture;
uniform sampler2D ptopoTexture;

uniform vec3 v3CameraPos;
uniform vec3 v3LightPos;
uniform float fInnerRadius;
uniform float near;
uniform float far;

float fOuterRadius = fInnerRadius * 1.025;
float fOuterRadius2 = fOuterRadius*fOuterRadius;
float fInnerRadius2 = fInnerRadius*fInnerRadius;
float fKrESun = 0.0025 * 15.0;
float fKmESun = 0.0015 * 15.0;
float fKr4PI = 0.0025 * 4.0 * 3.142;
float fKm4PI = 0.0015 * 4.0 * 3.142;
float fScale = 1.0 / (fOuterRadius - fInnerRadius);
float fScaleDepth = 0.25;
float fScaleOverScaleDepth = (1.0 / (fOuterRadius - fInnerRadius)) / fScaleDepth;
int nSamples = 2;
float fSamples = 2.0;

float scale(float fCos);
float getNearIntersection(vec3 pos, vec3 ray, float distance2, float radius2);

float scale(float fCos) {
    float x = 1.0 - fCos;
    return fScaleDepth * exp(-0.00287 + x*(0.459 + x*(3.83 + x*(-6.80 + x*5.25))));
}

float getNearIntersection(vec3 pos, vec3 ray, float distance2, float radius2) {
    float B = 2.0 * dot(pos, ray);
    float C = distance2 - radius2;
    float det = max(0.0, B*B - 4.0 * C);
    return 0.5 * (-B - sqrt(det));
}

void main() {
    if (length(gl_Vertex.xyz) < 0.2) {
        vertex = vec4(normalize(gl_Vertex.xyz) * (fInnerRadius/2.0), 1.0);
        vvertex = gl_ModelViewMatrix * vertex;
        gl_Position = gl_ModelViewProjectionMatrix * vertex;
        gl_TexCoord[6] = gl_Position;
        return;
    }
    gl_TexCoord[0] = gl_MultiTexCoord0;
    
    vec4 heightmap = texture2D(topoTexture, gl_TexCoord[0].st);
    float height = heightmap.a*65536.0 + heightmap.r*256.0 - 32768.0;
    /*
    if (weight > 0.0) {
        vec2 oldtcoord = gl_TexCoord[0].st;
        oldtcoord.t = oldtcoord.t;
        vec2 parentCoords;
        if (index == 1)
            parentCoords = 1.0/texturesize+vec2(oldtcoord.s/2.0, oldtcoord.t/2.0);
        if (index == 2)
            parentCoords = vec2(0.5-1.0/texturesize+oldtcoord.s/2.0, 1.0/texturesize+oldtcoord.t/2.0);
        if (index == 3)
            parentCoords = vec2(1.0/texturesize+oldtcoord.s/2.0, 0.5-1.0/texturesize+oldtcoord.t/2.0);
        if (index == 4)
            parentCoords = 0.5-1.0/texturesize+vec2(oldtcoord.s/2.0, oldtcoord.t/2.0);
        vec4 pheightmap = texture2D(ptopoTexture, parentCoords);
        float pheight = pheightmap.a*65536.0 + pheightmap.r*256.0 - 32768.0;
        height = mix(height, pheight, weight);
    }
    */
    vertex = vec4(normalize(gl_Vertex.xyz) * (height + fInnerRadius), 1.0);
    vvertex = gl_ModelViewMatrix * vertex;
    gl_Position = gl_ModelViewProjectionMatrix * vertex;

    // prepoare
    const vec3  Wavelength = vec3(0.650,0.570,0.475);
    const vec3  v3InvWavelength = 1.0 / vec3(
        Wavelength.x * Wavelength.x * Wavelength.x * Wavelength.x,
        Wavelength.y * Wavelength.y * Wavelength.y * Wavelength.y, 
        Wavelength.z * Wavelength.z * Wavelength.z * Wavelength.z);

    float fCameraHeight = length(v3CameraPos);
    float fCameraHeight2 = fCameraHeight*fCameraHeight;

    vec3 v3Pos = vertex.xyz;
    vec3 v3Ray = v3Pos - v3CameraPos;
    float fFar = length(v3Ray);
    v3Ray /= fFar;

    float fNear, fDepth;
    vec3 v3Start;

    if (length(v3CameraPos) > fOuterRadius) {
        fNear = getNearIntersection(v3CameraPos, v3Ray, fCameraHeight2, fOuterRadius2);
        v3Start = v3CameraPos + v3Ray * fNear;
        fFar -= fNear;
        fDepth = exp((fInnerRadius - fOuterRadius) / fScaleDepth);
    } else {
        v3Start = v3CameraPos;
        fDepth = exp((fInnerRadius - fCameraHeight) / fScaleDepth);
    }

    float fCameraAngle = dot(-v3Ray, v3Pos) / length(v3Pos);
    float fLightAngle = dot(v3LightPos, v3Pos) / length(v3Pos);
    float fCameraScale = scale(fCameraAngle);
    float fLightScale = scale(fLightAngle);
    float fCameraOffset = fDepth*fCameraScale;
    float fTemp = (fLightScale + fCameraScale);

    float fSampleLength = fFar / fSamples;
    float fScaledLength = fSampleLength * fScale;
    vec3 v3SampleRay = v3Ray * fSampleLength;
    vec3 v3SamplePoint = v3Start + v3SampleRay * 0.5;

    // Now loop through the sample rays
    vec3 v3FrontColor = vec3(0.0, 0.0, 0.0);
    vec3 v3Attenuate;
    for (int i=0; i<nSamples; i++) {
        float fHeight = length(v3SamplePoint);
        float fDepth = exp(fScaleOverScaleDepth * (fInnerRadius - fHeight));
        float fScatter = fDepth*fTemp - fCameraOffset;
        v3Attenuate = exp(-fScatter * (v3InvWavelength * fKr4PI + fKm4PI));
        v3FrontColor += v3Attenuate * (fDepth * fScaledLength);
        v3SamplePoint += v3SampleRay;
    }
    gl_FrontColor.rgb = v3FrontColor * (v3InvWavelength * fKrESun + fKmESun);
    gl_FrontSecondaryColor.rgb = v3Attenuate;
    gl_TexCoord[6] = gl_Position;
}

#endif

#ifdef _FRAGMENT_

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
uniform float near;

uniform vec3 v3LightPos;

void main() {
    gl_FragDepth = (log(near * gl_TexCoord[6].z) / log(near * far));
    vec3 normal = texture2D(normalTexture, gl_TexCoord[0].st).xyz;
    vec4 color = texture2D(colorTexture, gl_TexCoord[0].st);

    // fetch normals and rotate
    normal = (normal * 2.0) - 1.0;
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
    /*
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
    */

    // lighting stuff
    vec3 s = normalize(vec3(gl_LightSource[0].position - vvertex));

    // finals
    vec4 ambient = vec4(0.05);
    vec4 diffuse = vec4(0.0);
    vec4 specular = vec4(0.0);

    // diffuse component
    float diffuse_coeff = max(0.0, dot(normal, s));
    if (diffuse_coeff > 0.0) {
        diffuse = vec4(0.8, 0.8, 0.8, 1.0) * diffuse_coeff;
        
        // specular component
        vec3 r = -reflect(s, normal);
        vec3 v = normalize(-vvertex.xyz);
        specular = vec4(0.4, 0.4, 0.4, 1.0) * pow(max(dot(r, v), 0.0), 8.0) * color.a;
    }

    gl_FragColor = color * (ambient + diffuse) + specular + (gl_Color + 0.25 * gl_SecondaryColor);
    gl_FragColor.a = 1.0;
}

#endif
