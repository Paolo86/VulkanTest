#version 450


layout(location = 0) in vec3 position;
layout(location = 1) in vec3 color;
layout(location = 2) in vec2 tex;
layout(location = 3) in vec3 normal;


layout(set = 1, binding = 0) uniform UboViewProjection {
 mat4 projection;
 mat4 view;	
 vec3 camPosition;
} uboViewProjection;



layout(push_constant) uniform PushModel {

	mat4 model;
} pushModel;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec2 fragTex;
layout(location = 2) out vec3 outNormal;
layout(location = 3) out vec3 fragPosition;
layout(location = 4) out vec3 viewPos;

void main() {

	vec4 fragPos = pushModel.model * vec4(position, 1.0);
	fragPosition = fragPos.xyz;
    gl_Position = uboViewProjection.projection * uboViewProjection.view * fragPos;
	fragColor = color;
	fragTex = tex;
	viewPos = uboViewProjection.camPosition;
	outNormal = vec3(pushModel.model * vec4(normal,0.0));

}