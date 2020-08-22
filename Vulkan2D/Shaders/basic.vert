#version 450


layout(location = 0) in vec3 position;
layout(location = 1) in vec3 color;
layout(location = 2) in vec2 tex;
layout(location = 3) in vec3 normal;
layout(location = 4) in vec3 tangent;
layout(location = 5) in vec3 binormal;

layout(location = 6) in vec3 instancePosition;
layout(location = 7) in vec3 instanceRotation;
layout(location = 8) in vec3 instanceScale;


layout(set = 1, binding = 0) uniform UboViewProjection {
 mat4 projection;
 mat4 view;	
} uboViewProjection;


layout(push_constant) uniform PushModel {

	mat4 model;
} pushModel;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec2 fragTex;

void main() {

    gl_Position = uboViewProjection.projection * uboViewProjection.view  * vec4(position + instancePosition, 1.0);
	fragColor = color;
	fragTex = tex;

}