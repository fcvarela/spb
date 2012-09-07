uniform sampler2D topoTexture;
uniform float lonspan;
uniform float size;

void main() {
  float height = 1.0;
  float step = 1.0/(size+4.0);

  float metersPerDegree = 30336.0;//(2.0 * PI * 1738140.0)/360.0;
  float resInMeters = (lonspan / size) * metersPerDegree;
  float bumpheight = 1.0 / resInMeters;

  vec2 l = vec2(gl_TexCoord[0].s - step, gl_TexCoord[0].t);
  vec2 r = vec2(gl_TexCoord[0].s + step, gl_TexCoord[0].t);
  vec2 u = vec2(gl_TexCoord[0].s, gl_TexCoord[0].t + step);
  vec2 d = vec2(gl_TexCoord[0].s, gl_TexCoord[0].t - step);
  vec2 c = gl_TexCoord[0].st;

  float heightl = texture2D(topoTexture, l).a*65536.0+texture2D(topoTexture, l).r*256.0+texture2D(topoTexture, l).g-32768.0;
  float heightr = texture2D(topoTexture, r).a*65536.0+texture2D(topoTexture, r).r*256.0+texture2D(topoTexture, r).g-32768.0;
  float heightu = texture2D(topoTexture, u).a*65536.0+texture2D(topoTexture, u).r*256.0+texture2D(topoTexture, u).g-32768.0;
  float heightd = texture2D(topoTexture, d).a*65536.0+texture2D(topoTexture, d).r*256.0+texture2D(topoTexture, d).g-32768.0;
  float heightc = texture2D(topoTexture, c).a*65536.0+texture2D(topoTexture, c).r*256.0+texture2D(topoTexture, c).g-32768.0;
  
  float pl = heightl*bumpheight;
  float pr = heightr*bumpheight;
  float pu = heightu*bumpheight;
  float pd = heightd*bumpheight;
  float pc = heightc*bumpheight;

  vec3 normal;
  normal = normalize(vec3(pu-pc, pc-pl, 1.0));
  normal+= normalize(vec3(pc-pd, pc-pl, 1.0));
  normal+= normalize(vec3(pc-pd, pr-pc, 1.0));
  normal+= normalize(vec3(pu-pc, pr-pc, 1.0));
  normal = normalize(normal);

  gl_FragColor.rgb = (normal + 1.0) / 2.0;
  gl_FragColor.a = 1.0;
}