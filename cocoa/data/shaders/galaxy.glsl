#ifdef _VERTEX_

uniform vec3 cameraPos;

void main(void) {
	gl_TexCoord[0] = gl_MultiTexCoord0;
	gl_Position = ftransform();
	
	float dist = length(gl_Vertex.xyz - cameraPos);
	gl_PointSize = 4.0 + (2.0 * 1280.0) / dist;
	gl_FrontColor = gl_Color;
}

#endif

#ifdef _FRAGMENT_

uniform sampler2D texture;

void main(void) {
	gl_FragColor = gl_Color * texture2D(texture, gl_TexCoord[0].st);
	//gl_FragColor.a = 1.0 - length(gl_TexCoord[0].st - vec2(0.5, 0.5))*2.0;
}

#endif
