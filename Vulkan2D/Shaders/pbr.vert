#version 450


layout(location = 0) in vec3 position;
layout(location = 1) in vec3 color;
layout(location = 2) in vec2 tex;
layout(location = 3) in vec3 normal;
layout(location = 4) in vec3 tangent;
layout(location = 5) in vec3 binormal;


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
layout(location = 2) out vec3 vertexNormal;
layout(location = 3) out vec3 fragPosition;
layout(location = 4) out vec3 viewPos;
layout(location = 5) out mat3 TBNout;

void main() {

	vec3 T = normalize(vec3(pushModel.model * vec4(normalize(tangent),   0.0)));
	vec3 B = normalize(vec3(pushModel.model * vec4(normalize(binormal), 0.0)));
	vec3 N = normalize(vec3(pushModel.model * vec4(normalize(normal),    0.0)));
	mat3 TBN = mat3(T, B, N);

	vec4 fragPos = pushModel.model * vec4(position, 1.0);
	fragPosition = fragPos.xyz;
    gl_Position = uboViewProjection.projection * uboViewProjection.view * fragPos;
	
	fragColor = normalize(normal);
	fragTex = tex;
	viewPos = uboViewProjection.camPosition;
	vertexNormal = vec3(pushModel.model * vec4(normal,0.0));
	TBNout = TBN;

}