varying vec4 vertex;
varying vec4 vvertex;

uniform sampler2D topoTexture;
uniform vec3 v3CameraPos;
uniform vec3 v3LightPos;

const float fInnerRadius = 1.0;
const float fOuterRadius = fInnerRadius * 1.025;
const float fOuterRadius2 = fOuterRadius*fOuterRadius;
const float fInnerRadius2 = fInnerRadius*fInnerRadius;
const float fKrESun = 0.0025 * 15.0;
const float fKmESun = 0.0015 * 15.0;
const float fKr4PI = 0.0025 * 4.0 * 3.142;
const float fKm4PI = 0.0015 * 4.0 * 3.142;
const float fScale = 1.0 / (fOuterRadius - fInnerRadius);
const float fScaleDepth = 0.25;
const float fScaleOverScaleDepth = (1.0 / (fOuterRadius - fInnerRadius)) / fScaleDepth;
const int nSamples = 2;
const float fSamples = 2.0;

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
    gl_TexCoord[0] = gl_MultiTexCoord0;
    
    float radius = 1738140.0;
    
    vec4 heightmap = texture2D(topoTexture, gl_TexCoord[0].st);
    float height = heightmap.a*256.0*256.0 + heightmap.r*256.0;
    height -= 16384.0;
    vertex = vec4(normalize(gl_Vertex.xyz) * (height + radius), 1.0);

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

    vec3 v3Pos = gl_Vertex.xyz;
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
}
