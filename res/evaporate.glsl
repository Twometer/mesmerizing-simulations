#version 430

uniform float evapSpeed;

layout (binding = 0, rgba32f) uniform image2D destTex;
layout (local_size_x = 16, local_size_y = 16) in;

void main() {
    ivec2 storePos = ivec2(gl_GlobalInvocationID.xy);
    vec4 px = imageLoad(destTex, storePos);
    px.rgb -= vec3(evapSpeed, evapSpeed, evapSpeed);
    imageStore(destTex, storePos, px);
}