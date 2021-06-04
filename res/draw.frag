#version 430

out vec4 color;

in vec2 texCoords;

uniform sampler2D textureSampler;

uniform vec3 darkColor;
uniform vec3 brightColor;

void main() {
    float val = texture(textureSampler, texCoords).r;
    vec3 inColor = mix(darkColor, brightColor, val * val) * val;
    color = vec4(inColor, 1.0);
}