#version 430

out vec4 color;

in vec2 texCoords;

uniform sampler2D textureSampler;
uniform vec3 colorMask;

void main() {
    color = vec4(texture(textureSampler, texCoords).rgb * colorMask, 1.0);
}