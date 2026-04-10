#version 430 core

// UV coordinates interpolated across the screen quad (0,0) to (1,1)
in vec2 v_uv;

// Final output color written to the framebuffer
out vec4 fragColor;

// Accumulation texture: holds the SUM of all light samples (RGB) and count (A)
uniform sampler2D u_accumulation_tex;

uniform uint u_tonemap;

uniform sampler3D u_lut_tex;
uniform int u_lut_size;

vec3 linearToSRGB(vec3 x) {
    x = clamp(x, 0.0, 1.0);
    return mix(
        12.92 * x,
        1.055 * pow(x, vec3(1.0 / 2.4)) - 0.055,
        step(vec3(0.0031308), x)
    );
}

vec3 applyLUT(vec3 color) {
    if (u_lut_size == 0)
        return color;
    float scale = float(u_lut_size - 1) / float(u_lut_size);
    float l_offset = 0.5 / float(u_lut_size);
    color = max(color, vec3(0.0));
    vec3 uvw = color * scale + l_offset;
    return texture(u_lut_tex, uvw).rgb;
}

void main()
{
    // Raw sum of all color samples accumulated so far at this pixel
    vec4 accum = texture(u_accumulation_tex, v_uv);

    // Clamp to 1.0 minimum to avoid division by zero on the first frame
    float sample_count = max(accum.a, 1.0);
    // Divide the accumulated sum by the frame count to get the running average.
    // The more frames accumulate, the more noise cancels out and the image converges.
    vec3 color = accum.rgb / sample_count;

	if (u_tonemap == 1 && u_lut_size > 0)
	{
		    color = linearToSRGB(color);
	      color = applyLUT(color);
	}
	else
		color = linearToSRGB(color);

    fragColor = vec4(color, 1.0);
}
