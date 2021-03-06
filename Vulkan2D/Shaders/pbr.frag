#version 450
#extension GL_ARB_separate_shader_objects : enable


layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragTex;
layout(location = 2) in vec3 vertexNormal;
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
// 0 - Albedo
// 1 - Normal map
// 2 - Metallic map
// 3 - Roughness map
// 4 - AO Map
layout(set=2, binding=0) uniform sampler2D textureSampler[5];

layout(set=2, binding=1) uniform MatProperties
{
	vec4 tint;
	vec2 uvScale;
	vec3 pbrProps;
} matProps;



layout(location = 0) out vec4 outColor;

float PI = 3.14159265359;
float DistributionGGX(vec3 N, vec3 H, float roughness);
float GeometrySchlickGGX(float NdotV, float roughness);
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness);
vec3 fresnelSchlick(float cosTheta, vec3 F0);
vec3 getNormalFromNormalMap();

void main() {
	
	vec3 albedo     = pow(texture(textureSampler[0], fragTex).rgb, vec3(2.2));
	float metallic  =  texture(textureSampler[2], fragTex).r + matProps.pbrProps.r;
    float roughness =  texture(textureSampler[3], fragTex).r + matProps.pbrProps.g;
    float ao        =  texture(textureSampler[4], fragTex).r + matProps.pbrProps.b;

	vec3 normal     = getNormalFromNormalMap();
	
	vec3 N = normalize(normal);
    vec3 V = normalize(viewPos - fragPosition);
		
	vec3 F0 = vec3(0.04); 
    F0 = mix(F0, albedo, metallic);

	// reflectance equation
    vec3 Lo = vec3(0.0);
	
	for(int i=0; i< dirLightsOut.misc.g; i++)
	{
		vec3 L = normalize(-dirLightsOut.direction[i].rgb);
		vec3 H = normalize(V + L);
		vec3 radiance     = dirLightsOut.diffuseColor[i].rgb *  dirLightsOut.intensity[i].r;
		
		float NDF = DistributionGGX(N, H, roughness);
		float G   = GeometrySmith(N, V, L, roughness);      
        vec3 F    = fresnelSchlick(max(dot(H, V), 0.0), F0);
		
		vec3 kS = F;
        vec3 kD = vec3(1.0) - kS;
        kD *= 1.0 - metallic;
		
		vec3 numerator    = NDF * G * F;
        float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0);
        vec3 specular     = numerator / max(denominator, 0.001);
		
		float NdotL = max(dot(N, L), 0.0);                
        Lo += (kD * albedo / PI + specular) * radiance * NdotL;
	}
	
	vec3 ambient = vec3(0.03) * albedo * ao;
    vec3 color = ambient + Lo;
	
    //color = color / (color + vec3(1.0));
    //color = pow(color, vec3(1.0/2.2));

	outColor = vec4(color, 1.0) * matProps.tint;


}


vec3 getNormalFromNormalMap()
{
	vec3 normal = texture(textureSampler[1], fragTex).rgb;
	normal = normal * 2.0 - 1.0;   
	normal = normalize (normal); 
	return normal;
}

float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a      = roughness*roughness;
    float a2     = a*a;
    float NdotH  = max(dot(N, H), 0.0);
    float NdotH2 = NdotH*NdotH;
	
    float num   = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;
	
    return num / denom;
}

vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}

float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r*r) / 8.0;

    float num   = NdotV;
    float denom = NdotV * (1.0 - k) + k;
	
    return num / denom;
}
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2  = GeometrySchlickGGX(NdotV, roughness);
    float ggx1  = GeometrySchlickGGX(NdotL, roughness);
	
    return ggx1 * ggx2;
}