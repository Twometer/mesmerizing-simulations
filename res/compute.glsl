#version 430

uniform writeonly image2D destTex;
layout (local_size_x = 16, local_size_y = 16) in;

void main() {
    ivec2 storePos = ivec2(gl_GlobalInvocationID.xy);
    imageStore(destTex, storePos, vec4(1, 0, 0, 1));
}