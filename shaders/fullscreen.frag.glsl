#version 430 core

// UV coordinates interpolated across the screen quad (0,0) to (1,1)
in vec2 v_uv;

// Final output color written to the framebuffer
out vec4 fragColor;

// Accumulation texture: holds the SUM of all light samples across every frame so far
uniform sampler2D u_accumulation_tex;

// How many frames have been rendered and added to the accumulation texture
uniform uint u_frame_index;

void main()
{
    // Raw sum of all color samples accumulated so far at this pixel
    vec4 accum = texture(u_accumulation_tex, v_uv);

    // Clamp to 1.0 minimum to avoid division by zero on the first frame
    float frame = max(float(u_frame_index), 1.0);

    // Divide the accumulated sum by the frame count to get the running average.
    // The more frames accumulate, the more noise cancels out and the image converges.
    vec3 color = accum.rgb / frame;

    // Apply gamma correction (linear -> sRGB).
    // Rendering math operates in linear light space, but monitors expect gamma-encoded
    // values. Raising to the power of 1/2.2 makes the image look correct on screen.
    color = pow(color, vec3(1.0 / 2.2));

    // Output the final averaged, gamma-corrected color with full opacity
    fragColor = vec4(color, 1.0);
}
