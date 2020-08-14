#version 450


layout(location = 0) in vec3 position;
layout(location = 1) in vec3 color;
layout(location = 2) in vec2 tex;


layout(set = 0, binding = 0) uniform UboViewProjection {
 mat4 projection;
 mat4 view;	
} uboViewProjection;

layout(set = 0, binding = 1) uniform DUboViewProjection {
 mat4 projection;
 mat4 view;	
} dynamicUBO;

layout(push_constant) uniform PushModel {

	mat4 model;
} pushModel;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec2 fragTex;

void main() {

    gl_Position = uboViewProjection.projection * uboViewProjection.view * pushModel.model * vec4(position, 1.0);
	fragColor = color;
	fragTex = tex;

}