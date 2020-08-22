#version 450
#extension GL_ARB_separate_shader_objects : enable


layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragTex;
layout(location = 2) in vec3 outNormal;
layout(location = 3) in vec3 fragPosition;
layout(location = 4) in vec3 viewPos;
layout(location = 5) in DirectionalLightsOut
{
	vec4 diffuseColor[4];
	vec4 specularColor[4];
	vec4 direction[4];
	vec4 intensity[4];
	vec4 misc;
} dirLightsOut;

// Set 2, Material
layout(set=2, binding=0) uniform sampler2D textureSampler[5];

layout(set=2, binding=1) uniform MatProperties
{
	vec4 tint;
	vec2 uvScale;
	vec3 pbrProps;
} matProps;




layout(location = 0) out vec4 outColor;

void main() {
	
	vec3 viewDir = normalize(viewPos - fragPosition);
		
	vec4 totalDirectional = vec4(0,0,0,0);
	for(int i=0; i< dirLightsOut.misc.g; i++)
	{
		float diffuseDot = max(dot(normalize(-dirLightsOut.direction[i].rgb),normalize(outNormal)), 0.0);
		vec4 finalColor = vec4(dirLightsOut.diffuseColor[i].rgb * diffuseDot, 1.0) * dirLightsOut.intensity[i].r;
		
		vec3 reflectDir = reflect(normalize(dirLightsOut.direction[i].rgb), normalize(outNormal));
		float viewDot = dot(normalize(viewDir), normalize(reflectDir));
		float spec = pow(max(viewDot, 0.0), 32);
		vec4 specular = spec * dirLightsOut.specularColor[i]* dirLightsOut.intensity[i].r;
		
		totalDirectional += (finalColor + specular);
	}
		

    outColor =  texture(textureSampler[0], fragTex * matProps.uvScale) * matProps.tint  * totalDirectional;
}