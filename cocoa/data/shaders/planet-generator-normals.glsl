#ifdef _VERTEX_

void main() {
    gl_TexCoord[0] = gl_MultiTexCoord0;
    gl_Position = ftransform();
}

#endif

#ifdef _FRAGMENT_

uniform sampler2D positionTexture;
uniform sampler2D topoTexture;
uniform float size;

void main() {
  float height = 1.0;
  float topostep = 1.0/size;

  // neighbor texture coordinates
  vec2 l = vec2(gl_TexCoord[0].s - topostep, gl_TexCoord[0].t);
  vec2 r = vec2(gl_TexCoord[0].s + topostep, gl_TexCoord[0].t);
  vec2 u = vec2(gl_TexCoord[0].s, gl_TexCoord[0].t + topostep);
  vec2 d = vec2(gl_TexCoord[0].s, gl_TexCoord[0].t - topostep);
  vec2 c = gl_TexCoord[0].st;

  vec3 vl = texture2D(positionTexture, l).xyz;
  vec3 vr = texture2D(positionTexture, r).xyz;
  vec3 vu = texture2D(positionTexture, u).xyz;
  vec3 vd = texture2D(positionTexture, d).xyz;
  vec3 vc = texture2D(positionTexture, c).xyz;
  
  float heightl = texture2D(topoTexture, l).a*65536.0+texture2D(topoTexture, l).r*256.0+texture2D(topoTexture, l).g-32768.0;
  float heightr = texture2D(topoTexture, r).a*65536.0+texture2D(topoTexture, r).r*256.0+texture2D(topoTexture, r).g-32768.0;
  float heightu = texture2D(topoTexture, u).a*65536.0+texture2D(topoTexture, u).r*256.0+texture2D(topoTexture, u).g-32768.0;
  float heightd = texture2D(topoTexture, d).a*65536.0+texture2D(topoTexture, d).r*256.0+texture2D(topoTexture, d).g-32768.0;
  float heightc = texture2D(topoTexture, c).a*65536.0+texture2D(topoTexture, c).r*256.0+texture2D(topoTexture, c).g-32768.0;
  
  vl *= (1738140.0 + heightl);
  vr *= (1738140.0 + heightr);
  vu *= (1738140.0 + heightu);
  vd *= (1738140.0 + heightd);
  vc *= (1738140.0 + heightc);

  vec3 v1 = vu - vc;
  vec3 v2 = vl - vc;
  vec3 v3 = vd - vc;
  vec3 v4 = vr - vc;

  vec3 normal = vec3(0.0);
  normal += cross(v1, v2);
  /*normal += cross(v2, v3);
  normal += cross(v3, v4);
  normal += cross(v4, v1);*/
  normal = normalize(normal);
  
  gl_FragColor.rgb = (normal + 1.0) / 2.0;
  gl_FragColor.a = 1.0;
}

#endif
