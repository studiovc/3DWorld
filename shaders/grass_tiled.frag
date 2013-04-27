uniform sampler2D tex0;
varying vec2 tc;

void main()
{
	if (gl_Color.a == 0.0) discard;
	gl_FragColor = apply_fog(gl_Color*texture2D(tex0, tc));
}
