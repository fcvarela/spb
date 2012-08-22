vec4 mod289(vec4 x) {
  return x - floor(x * (1.0 / 289.0)) * 289.0; }

float mod289(float x) {
  return x - floor(x * (1.0 / 289.0)) * 289.0; }

vec4 permute(vec4 x) {
     return mod289(((x*34.0)+1.0)*x);
}

float permute(float x) {
     return mod289(((x*34.0)+1.0)*x);
}

vec4 taylorInvSqrt(vec4 r) {
  return 1.79284291400159 - 0.85373472095314 * r;
}

float taylorInvSqrt(float r) {
  return 1.79284291400159 - 0.85373472095314 * r;
}

vec4 grad4(float j, vec4 ip) {
  const vec4 ones = vec4(1.0, 1.0, 1.0, -1.0);
  vec4 p,s;

  p.xyz = floor( fract (vec3(j) * ip.xyz) * 7.0) * ip.z - 1.0;
  p.w = 1.5 - dot(abs(p.xyz), ones.xyz);
  s = vec4(lessThan(p, vec4(0.0)));
  p.xyz = p.xyz + (s.xyz*2.0 - 1.0) * s.www; 

  return p;
}

// (sqrt(5) - 1)/4 = F4, used once below
#define F4 0.309016994374947451

float snoise(vec4 v) {
  const vec4  C = vec4( 0.138196601125011,  // (5 - sqrt(5))/20  G4
                        0.276393202250021,  // 2 * G4
                        0.414589803375032,  // 3 * G4
                       -0.447213595499958); // -1 + 4 * G4

// First corner
  vec4 i  = floor(v + dot(v, vec4(F4)) );
  vec4 x0 = v -   i + dot(i, C.xxxx);

// Other corners

// Rank sorting originally contributed by Bill Licea-Kane, AMD (formerly ATI)
  vec4 i0;
  vec3 isX = step( x0.yzw, x0.xxx );
  vec3 isYZ = step( x0.zww, x0.yyz );
//  i0.x = dot( isX, vec3( 1.0 ) );
  i0.x = isX.x + isX.y + isX.z;
  i0.yzw = 1.0 - isX;
//  i0.y += dot( isYZ.xy, vec2( 1.0 ) );
  i0.y += isYZ.x + isYZ.y;
  i0.zw += 1.0 - isYZ.xy;
  i0.z += isYZ.z;
  i0.w += 1.0 - isYZ.z;

  // i0 now contains the unique values 0,1,2,3 in each channel
  vec4 i3 = clamp( i0, 0.0, 1.0 );
  vec4 i2 = clamp( i0-1.0, 0.0, 1.0 );
  vec4 i1 = clamp( i0-2.0, 0.0, 1.0 );

  //  x0 = x0 - 0.0 + 0.0 * C.xxxx
  //  x1 = x0 - i1  + 1.0 * C.xxxx
  //  x2 = x0 - i2  + 2.0 * C.xxxx
  //  x3 = x0 - i3  + 3.0 * C.xxxx
  //  x4 = x0 - 1.0 + 4.0 * C.xxxx
  vec4 x1 = x0 - i1 + C.xxxx;
  vec4 x2 = x0 - i2 + C.yyyy;
  vec4 x3 = x0 - i3 + C.zzzz;
  vec4 x4 = x0 + C.wwww;

// Permutations
  i = mod289(i); 
  float j0 = permute( permute( permute( permute(i.w) + i.z) + i.y) + i.x);
  vec4 j1 = permute( permute( permute( permute (
             i.w + vec4(i1.w, i2.w, i3.w, 1.0 ))
           + i.z + vec4(i1.z, i2.z, i3.z, 1.0 ))
           + i.y + vec4(i1.y, i2.y, i3.y, 1.0 ))
           + i.x + vec4(i1.x, i2.x, i3.x, 1.0 ));

// Gradients: 7x7x6 points over a cube, mapped onto a 4-cross polytope
// 7*7*6 = 294, which is close to the ring size 17*17 = 289.
  vec4 ip = vec4(1.0/294.0, 1.0/49.0, 1.0/7.0, 0.0) ;

  vec4 p0 = grad4(j0,   ip);
  vec4 p1 = grad4(j1.x, ip);
  vec4 p2 = grad4(j1.y, ip);
  vec4 p3 = grad4(j1.z, ip);
  vec4 p4 = grad4(j1.w, ip);

// Normalise gradients
  vec4 norm = taylorInvSqrt(vec4(dot(p0,p0), dot(p1,p1), dot(p2, p2), dot(p3,p3)));
  p0 *= norm.x;
  p1 *= norm.y;
  p2 *= norm.z;
  p3 *= norm.w;
  p4 *= taylorInvSqrt(dot(p4,p4));

// Mix contributions from the five corners
  vec3 m0 = max(0.6 - vec3(dot(x0,x0), dot(x1,x1), dot(x2,x2)), 0.0);
  vec2 m1 = max(0.6 - vec2(dot(x3,x3), dot(x4,x4)            ), 0.0);
  m0 = m0 * m0;
  m1 = m1 * m1;
  return 49.0 * ( dot(m0*m0, vec3( dot( p0, x0 ), dot( p1, x1 ), dot( p2, x2 )))
               + dot(m1*m1, vec2( dot( p3, x3 ), dot( p4, x4 ) ) ) ) ;

  }

float ridge(float h, float offset) {
    h = abs(h);
    h = offset - h;
    h = h * h;
    return h;
}

float ridgedmf(vec4 p/*, float H, float lacunarity, int octaves, float offset, int sharpness, int threshold, float K*/)
{
  float H = 0.8;
  float lacunarity = 1.5;
  int octaves = 35;
  float offset = 0.80;
  float sharpness = 4.0;
  float threshold = 35.0;
  float Kt = 0.1;

  float result, signal, weight, exponent;
  for (int i=0; i<octaves; i += 1 ) {
          /* First octaves */

          if ( i == 0) {
              signal = snoise(p);
              if ( signal < 0.0 ) signal = -signal;
              signal = offset - signal;
              signal = pow( signal, sharpness );
                             /*This should give you a power function to control
                             sharpness of the ridges. Or you can just use the
                             original one -- signal *= signal;*/
              result = signal;
              weight = 1.0;
          }else{
          
              exponent = pow( lacunarity, (-float(i)*H) );
      /*
              PP.x *= lacunarity;
              PP.y *= lacunarity;
              PP.z *= lacunarity;
      */
        p = p * lacunarity;
              /* weigh successive contributions by previous signal */
              weight = signal * threshold;
              weight = clamp(weight , 0.0, 1.0);    
              signal = snoise(p);

              /* get absolute value of signal*/
              signal = abs(signal);

              /* invert and translate*/
              signal = offset - signal;

              /* sharpen the ridge*/
              signal = pow( signal, sharpness );

              /* weight the contribution*/
              signal *= weight;
              result += signal * exponent;
          }
  }

  return result;
}

uniform float baselat;
uniform float baselon;
uniform float latspan;
uniform float lonspan;

vec4 coords() {
  float lat = baselat + gl_TexCoord[0].t * latspan;
  float lon = baselon + gl_TexCoord[0].s * lonspan;

  // map to spherical coords
  vec3 coords;
  coords[2] = cos(lon) * cos(lat);
  coords[0] = sin(lon) * cos(lat);
  coords[1] = sin(lat);

  return vec4(coords, 1.0);
}

vec4 getColor(float height) {
  vec3 shades[10];
    float heights[10];
    float diffs[10];
    heights[0] = -16384.0;  shades[0] = vec3(  3,  29,  63);
    heights[1] = -256.0;    shades[1] = vec3(  3,  29,  63);
    heights[2] = -50.0;     shades[2] = vec3(  7, 106, 127);
    heights[3] = 0.0;       shades[3] = vec3( 62,  86,  30);
    heights[4] = 1024.0;    shades[4] = vec3( 84,  96,  50);
    heights[5] = 2048.0;    shades[5] = vec3( 62,  86,  30);
    heights[6] = 6000.0;    shades[6] = vec3(184, 163, 141);
    heights[7] = 8000.0;    shades[7] = vec3(130, 127,  97);
    heights[8] = 9000.0;    shades[8] = vec3(255, 255, 255);
    heights[9] = 16384.0;   shades[9] = vec3(128, 255, 255);

    vec3 shade;
    float scale = 0.0;
    float coeff = 0.0;

    for (int i=0; i<9; i++) {
        if (height > heights[i] && height <= heights[i+1]) {
            scale = smoothstep(heights[i], heights[i+1], height);
            shade = vec3(mix(shades[i], shades[i+1], scale))/255.0;
        }
    }
  
  return vec4(shade, 1.0);
}

void main() {
  // pipeline here [copy from complexplanet]
  float height = 0.0;
  height += ridgedmf(coords());
  gl_FragColor.a = height;
  gl_FragColor.rgb = getColor(height*16384.0-8192.0).rgb;
}
