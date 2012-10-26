#ifdef _VERTEX_

uniform vec3 cameraPos;

void main(void) {
	gl_TexCoord[0] = gl_MultiTexCoord0;
	gl_Position = ftransform();
	
	vec4 position = gl_ModelViewMatrix * gl_Vertex;
    vec4 camera = gl_ModelViewMatrix * vec4(cameraPos, 1.0);
	float dist = length(position-camera);
	gl_PointSize = 2.0 + (2.0 * 1280.0) / dist;
	gl_FrontColor = gl_Color;
}

#endif

#ifdef _FRAGMENT_

uniform sampler2D texture;

void main(void) {
	gl_FragColor = gl_Color * texture2D(texture, gl_TexCoord[0].st);
}

#endif
