#version 430

uniform writeonly image2D destTex;
layout (local_size_x = 16, local_size_y = 16) in;

highp float rand(vec2 co)
{
    highp float a = 12.9898;
    highp float b = 78.233;
    highp float c = 43758.5453;
    highp float dt= dot(co.xy ,vec2(a,b));
    highp float sn= mod(dt,3.14);
    return fract(sin(sn) * c);
}

void main() {
    ivec2 storePos = ivec2(gl_GlobalInvocationID.xy);
    float r = rand(storePos);

    imageStore(destTex, storePos, vec4(r, r, r, 1));
}