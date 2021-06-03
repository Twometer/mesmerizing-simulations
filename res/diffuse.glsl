#version 430

uniform float diffusionSpeed;
uniform float evapSpeed;

layout (binding = 0, rgba32f) uniform image2D destTex;
layout (local_size_x = 16, local_size_y = 16) in;

void main() {
    ivec2 position = ivec2(gl_GlobalInvocationID.xy);
    vec4 px = imageLoad(destTex, position);

    vec4 sum = vec4(0);
    for (int ox = -1; ox <= 1; ox++) {
        for (int oy = -1; oy <= 1; oy++) {
            int sx = position.x + ox;
            int sy = position.y + oy;
            sum += imageLoad(destTex, ivec2(sx, sy));
        }
    }

    vec4 diffused = mix(px, sum / 9, diffusionSpeed);
    vec4 evaporated = max(vec4(0), diffused - 0.01 * evapSpeed);

    imageStore(destTex, position, evaporated);
}