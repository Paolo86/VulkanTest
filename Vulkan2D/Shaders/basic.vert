#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 color;


layout(binding = 0) uniform UboViewProjection {
 mat4 projection;
 mat4 view;	
} uboViewProjection;

layout(binding = 1) uniform UboModel {
 mat4 model;	
} uboModel;

layout(push_constant) uniform PushModel {

	mat4 model;
} pushModel;

layout(location = 0) out vec3 fragColor;

void main() {

    gl_Position = uboViewProjection.projection * uboViewProjection.view * pushModel.model * vec4(position, 1.0);
	fragColor = color;

}