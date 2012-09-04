uniform sampler2D topoTexture;

void main() {
  vec4 topo = texture2D(topoTexture, gl_TexCoord[0].st);
  float height = topo.a*65536.0+topo.r*256.0-16384.0;
  
  vec4 shades[10];
  float heights[10];
  float diffs[10];
  heights[0] = -16384.0;  shades[0] = vec4(  3,  29,  63, 255);
  heights[1] = -256.0;    shades[1] = vec4(  3,  29,  63, 255);
  heights[2] = -50.0;     shades[2] = vec4(  7, 106, 127, 255);
  heights[3] = 0.0;       shades[3] = vec4( 62,  86,  30, 0);
  heights[4] = 1024.0;    shades[4] = vec4( 84,  96,  50, 0);
  heights[5] = 2048.0;    shades[5] = vec4( 62,  86,  30, 0);
  heights[6] = 6000.0;    shades[6] = vec4(184, 163, 141, 0);
  heights[7] = 8000.0;    shades[7] = vec4(130, 127,  97, 0);
  heights[8] = 9000.0;    shades[8] = vec4(255, 255, 255, 0);
  heights[9] = 16384.0;   shades[9] = vec4(128, 255, 255, 0);

  vec4 shade;
  float scale = 0.0;
  float coeff = 0.0;
  
  shade = shades[0]/255.0;
  for (int i=0; i<9; i++) {
    if (height > heights[i] && height <= heights[i+1]) {
      scale = smoothstep(heights[i], heights[i+1], height);
      shade = vec4(mix(shades[i], shades[i+1], scale))/255.0;
    }
  }
  
  gl_FragColor = shade;
}
