#version 450
#extension GL_ARB_separate_shader_objects : enable


layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragTex;

layout(set=1, binding=0) uniform sampler2D textureSampler[2];

layout(location = 0) out vec4 outColor;

void main() {
    outColor = mix(texture(textureSampler[0], fragTex),texture(textureSampler[1], fragTex),0.5);
}