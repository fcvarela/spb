uniform sampler2D topoTexture;

void main() {
  vec4 topo = texture2D(topoTexture, gl_TexCoord[0].st);
  float height = topo.a*256.0*256.0+topo.r*256.0-16384.0;
  
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
  
  shade = shades[0]/255.0;
  for (int i=0; i<9; i++) {
    if (height > heights[i] && height <= heights[i+1]) {
      scale = smoothstep(heights[i], heights[i+1], height);
      shade = vec3(mix(shades[i], shades[i+1], scale))/255.0;
    }
  }
  
  gl_FragColor.rgb = shade;
  gl_FragColor.a = 1.0;
}
