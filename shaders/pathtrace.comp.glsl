#version 430 core

// Each workgroup is an 8x8 block of threads (64 threads total).
// The CPU dispatches enough workgroups to cover the entire image,
// e.g. a 1920x1080 image needs ceil(1920/8) x ceil(1080/8) = 240x135 workgroups.
layout(local_size_x = 8, local_size_y = 8) in;

// Direct read/write access to the accumulation texture.
// rgba32f = 4 full 32-bit floats per pixel, necessary because accumulated
// values grow beyond the 0-1 range as frames add up.
// (Unlike sampler2D which is read-only and filtered)
layout(rgba32f, binding = 0) uniform image2D u_accumulation;

// How many frames have been accumulated so far
uniform uint u_frame_index;

// Width and height of the image in pixels, used for bounds checking and UV calculation
uniform vec2 u_resolution;

void main()
{
    // Each thread gets a unique pixel coordinate from its position in the dispatch grid.
    // gl_GlobalInvocationID is a built-in that uniquely identifies this thread
    // across all workgroups.
    ivec2 pixel = ivec2(gl_GlobalInvocationID.xy);

    // Because workgroups are dispatched in multiples of 8, the grid may extend
    // slightly beyond the actual image. Threads that fall outside just exit early
    // to avoid writing garbage into memory.
    if (pixel.x >= int(u_resolution.x) ||
        pixel.y >= int(u_resolution.y))
        return;

    // Convert integer pixel coordinates to normalized UVs in the 0-1 range.
    // The + 0.5 shifts to the CENTER of the pixel rather than its corner,
    // which is the mathematically correct sampling point.
    vec2 uv = (vec2(pixel) + 0.5) / u_resolution;

    // --- RAY COLOR GOES HERE ---
    // This is a placeholder for the actual path tracing calculation.
    // Currently it just outputs a color gradient (red = left->right, green = bottom->top).
    // In a real renderer this is where rays would be cast into the scene.
    vec3 color = vec3(uv.x, uv.y, 0.2);

    // Read the current accumulated sum for this pixel.
    // imageLoad uses integer pixel coordinates with no filtering,
    // unlike texture() which interpolates.
    vec4 previous = imageLoad(u_accumulation, pixel);

    // On the very first frame, treat the previous value as zero regardless of
    // what is in the texture. This restarts accumulation cleanly whenever the
    // scene or camera changes, without needing a separate CPU-side clear pass.
    if (u_frame_index == 0)
        previous = vec4(0.0);

    // Add this frame's color sample to the running total and write it back.
    // The fragment shader from the previous step will later divide this sum
    // by u_frame_index to compute the average and display it on screen.
    imageStore(u_accumulation, pixel, previous + vec4(color, 1.0));
}
