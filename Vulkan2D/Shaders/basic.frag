#version 450
#extension GL_ARB_separate_shader_objects : enable


layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragTex;

// Set 1, Material
layout(set=2, binding=0) uniform sampler2D textureSampler[5];

layout(set=2, binding=1) uniform MatProperties
{
	vec4 tint;
	vec2 uvScale;
} matProps;

layout(location = 0) out vec4 outColor;

void main() {
    outColor = texture(textureSampler[0], fragTex * matProps.uvScale) * matProps.tint;
}