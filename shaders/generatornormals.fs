uniform sampler2D topoTexture;
uniform float lonspan;
uniform float size;

void main() {
  float height = 1.0;
  float step = 1.0/size;

  float PI = 3.14159265358979323846264;
  float metersPerDegree = (2.0 * PI * 1738140.0)/360.0;
  float resInMeters = (lonspan / size) * metersPerDegree;
  float bumpheight = 1.0 / resInMeters;

  vec2 u = vec2(gl_TexCoord[0].s, gl_TexCoord[0].t + step);
  vec2 r = vec2(gl_TexCoord[0].s + step, gl_TexCoord[0].t + step);

  float nu = (texture2D(topoTexture, u).a*16384.0-8192.0)*bumpheight;
  float nr = (texture2D(topoTexture, r).a*16384.0-8192.0)*bumpheight;
  float nc = (texture2D(topoTexture, gl_TexCoord[0].st).a*16384.0-8192.0)*bumpheight;

  float ncr = (nc - nr);
  float ncu = (nc - nu);
  float d = sqrt ((ncu * ncu) + (ncr * ncr) + 1.0);
  float vxc = (nc - nr) / d;
  float vyc = (nc - nu) / d;
  float vzc = 1.0 / d;

  // Map the normal range from the (-1.0 .. +1.0) range to the (0 .. 255)
  // range.
  gl_FragColor.rgb = vec3(vxc+1.0, vyc+1.0, vzc+1.0)/2.0;
  gl_FragColor.a = 1.0;
}
