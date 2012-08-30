varying vec3 v3Pos;
uniform vec3 v3CameraPos;
uniform vec3 v3LightPos;

const float fInnerRadius = 1.0;
const float fOuterRadius = fInnerRadius * 1.025;
const float fOuterRadius2 = fOuterRadius * fOuterRadius;
const float fInnerRadius2 = fInnerRadius * fInnerRadius;
const float fKrESun = 0.0025 * 15.0;
const float fKmESun = 0.0015 * 15.0;
const float fKr4PI = 0.0025 * 4.0 * 3.14159;
const float fKm4PI = 0.0015 * 4.0 * 3.14159;
const float fScale = 1.0 / (fOuterRadius - fInnerRadius);
const float fScaleDepth = 0.25;
const float fScaleOverScaleDepth = (1.0 / (fOuterRadius - fInnerRadius)) / fScaleDepth;
const int nSamples = 3;
const float fSamples = 3.0;
const float g = -0.90;
const float g2 = 0.81;

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

void main (void) {
    // prepare
    const vec3 Wavelength = vec3(0.650,0.570,0.475);
    const vec3 v3InvWavelength = 1.0 / vec3(
        Wavelength.x * Wavelength.x * Wavelength.x * Wavelength.x,
        Wavelength.y * Wavelength.y * Wavelength.y * Wavelength.y, 
        Wavelength.z * Wavelength.z * Wavelength.z * Wavelength.z);

    float fCameraHeight = length(v3CameraPos);
    float fCameraHeight2 = fCameraHeight*fCameraHeight;

    vec3 v3Ray = v3Pos - v3CameraPos;
    float fFar = length(v3Ray);
    v3Ray /= fFar;

    float fStartOffset, fNear, fDepth;
    vec3 v3Start;

    if (length(v3CameraPos) > fOuterRadius) {
        fNear = getNearIntersection(v3CameraPos, v3Ray, fCameraHeight2, fOuterRadius2);
        v3Start = v3CameraPos + v3Ray * fNear;
        fFar -= fNear;
        float fStartAngle = dot(v3Ray, v3Start) / fOuterRadius;
        float fStartDepth = exp(-1.0 / fScaleDepth);
        fStartOffset = fStartDepth * scale(fStartAngle);
    } else {
        v3Start = v3CameraPos;
        float fHeight = length(v3Start);
        fDepth = exp(fScaleOverScaleDepth * (fInnerRadius - fCameraHeight));
        float fStartAngle = dot(v3Ray, v3Start) / fHeight;
        fStartOffset = fDepth*scale(fStartAngle);
    }

    // Initialize the scattering loop variables
    float fSampleLength = fFar / fSamples;
    float fScaledLength = fSampleLength * fScale;
    vec3 v3SampleRay = v3Ray * fSampleLength;
    vec3 v3SamplePoint = v3Start + v3SampleRay * 0.5;

    // Now loop through the sample rays
    vec3 v3FrontColor = vec3(0.0, 0.0, 0.0);
    for(int i=0; i<nSamples; i++) {
        float fHeight = length(v3SamplePoint);
        float fDepth = exp(fScaleOverScaleDepth * (fInnerRadius - fHeight));
        float fLightAngle = dot(v3LightPos, v3SamplePoint) / fHeight;
        float fCameraAngle = dot(v3Ray, v3SamplePoint) / fHeight;
        float fScatter = (fStartOffset + fDepth*(scale(fLightAngle) - scale(fCameraAngle)));
        vec3 v3Attenuate = exp(-fScatter * (v3InvWavelength * fKr4PI + fKm4PI));
        v3FrontColor += v3Attenuate * (fDepth * fScaledLength);
        v3SamplePoint += v3SampleRay;
    }

    // Finally, scale the Mie and Rayleigh colors and set up the varying variables for the pixel shader
    vec4 secondaryColor = vec4(v3FrontColor * fKmESun, 1.0);
    vec4 primaryColor = vec4(v3FrontColor * (v3InvWavelength * fKrESun), 1.0);
    vec3 v3Direction = v3CameraPos - v3Pos;

    float fCos = dot(v3LightPos, v3Direction) / length(v3Direction);
    float fRayleighPhase = 0.75 * (1.0 + fCos*fCos);
    float fMiePhase = 1.5 * ((1.0 - g2) / (2.0 + g2)) * (1.0 + fCos*fCos) / pow(1.0 + g2 - 2.0*g*fCos, 1.5);
    
    gl_FragColor = fRayleighPhase * primaryColor + fMiePhase * secondaryColor;
    gl_FragColor = 1.0 - exp(-2.0 * gl_FragColor);
}
