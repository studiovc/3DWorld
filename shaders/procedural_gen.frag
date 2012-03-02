uniform float min_alpha = 0.0;
uniform vec3 const_indir_color = vec3(0,0,0);

// clipped eye position, clipped vertex position
varying vec3 eye, vpos, normal; // world space
varying vec4 epos;
varying vec3 eye_norm;

void main()
{
	vec3 norm_normal = normalize(normal);
	vec4 texel  = lookup_triplanar_texture(vpos, norm_normal);
	float alpha = gl_Color.a;
	vec3 lit_color = gl_Color.rgb; // base color (with some lighting)
	lit_color += gl_FrontMaterial.diffuse.rgb * const_indir_color; // add constant indir
	
	// directional light sources with no attenuation (Note: could add other lights later)
	if (enable_light0)  lit_color += add_light_comp_pos_smap(normalize(eye_norm), epos, 0).rgb;
	if (enable_light1)  lit_color += add_light_comp_pos_smap(normalize(eye_norm), epos, 1).rgb;
	if (enable_dlights) lit_color += gl_FrontMaterial.diffuse.rgb * add_dlights(vpos, norm_normal, eye); // dynamic lighting
	vec4 color = vec4((texel.rgb * lit_color), (texel.a * alpha));
	if (color.a <= min_alpha) discard;
	gl_FragColor = apply_fog(color); // apply standard fog
}
