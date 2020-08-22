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
 vec4 camPosition;
} uboViewProjection;



layout(push_constant) uniform PushModel {

	mat4 model;
} pushModel;

layout(set=0, binding=0) uniform DirectionalLights
{

	vec4 diffuseColor[4];
	vec4 specularColor[4];
	vec4 direction[4];
	vec4 intensity[4];
	vec4 misc;
} dirLights;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec2 fragTex;
layout(location = 2) out vec3 vertexNormal;
layout(location = 3) out vec3 fragPosition;
layout(location = 4) out vec3 viewPos;
layout(location = 5) out DirectionalLightsOut
{
	vec4 diffuseColor[4];
	vec4 specularColor[4];
	vec4 direction[4];
	vec4 intensity[4];
	vec4 misc;
} dirLightsOut;


mat4 build_transform(vec3 pos, vec3 ang) 
{
  float cosX = cos(ang.x);
  float sinX = sin(ang.x);
  float cosY = cos(ang.y);
  float sinY = sin(ang.y);
  float cosZ = cos(ang.z);
  float sinZ = sin(ang.z);

  mat4 m;

  float m00 = cosY * cosZ + sinX * sinY * sinZ; 
  float m01 = cosY * sinZ - sinX * sinY * cosZ; 
  float m02 = cosX * sinY;
  float m03 = 0.0;
  
  float m04 = -cosX * sinZ; 
  float m05 = cosX * cosZ; 
  float m06 = sinX;
  float m07 = 0.0;
  
  float m08 = sinX * cosY * sinZ - sinY * cosZ;
  float m09 = -sinY * sinZ - sinX * cosY * cosZ;
  float m10 = cosX * cosY;
  float m11 = 0.0;
  
  float m12 = pos.x;
  float m13 = pos.y;
  float m14 = pos.z;
  float m15 = 1.0;

  /*
  //------ Orientation ---------------------------------
  m[0] = vec4(m00, m01, m02, m03); // first column.
  m[1] = vec4(m04, m05, m06, m07); // second column.
  m[2] = vec4(m08, m09, m10, m11); // third column.

  //------ Position ------------------------------------
  m[3] = vec4(m12, m13, m14, m15); // fourth column.
  */

  //------ Orientation ---------------------------------
  m[0][0] = m00; // first entry of the first column.
  m[0][1] = m01; // second entry of the first column.
  m[0][2] = m02;
  m[0][3] = m03;
  
  m[1][0] = m04; // first entry of the second column.
  m[1][1] = m05; // second entry of the second column.
  m[1][2] = m06;
  m[1][3] = m07;

  m[2][0] = m08; // first entry of the third column.
  m[2][1] = m09; // second entry of the third column.
  m[2][2] = m10;
  m[2][3] = m11;
  
  //------ Position ------------------------------------
  m[3][0] = m12; // first entry of the fourth column.
  m[3][1] = m13; // second entry of the fourth column.
  m[3][2] = m14;
  m[3][3] = m15;

  return m;
}


void main() {

	mat4 instancedModel = build_transform(instancePosition, instanceRotation);
	
	vec3 T = normalize(vec3(instancedModel * vec4(normalize(tangent),   0.0)));
	vec3 B = normalize(vec3(instancedModel * vec4(normalize(binormal), 0.0)));
	vec3 N = normalize(vec3(instancedModel * vec4(normalize(normal),    0.0)));
	mat3 TBN = transpose(mat3(T, B, N));

	vec3 fragPos = TBN * (instancedModel * vec4(position,1.0)).xyz;
	fragPosition = fragPos;
    gl_Position = uboViewProjection.projection * uboViewProjection.view * instancedModel * vec4(position, 1.0);

	fragColor = normalize(normal);
	fragTex = tex;
	viewPos = TBN * vec3(uboViewProjection.camPosition);
	vertexNormal = TBN * vec3(instancedModel * vec4(normal,0.0));
	
	for(int i=0; i< dirLights.misc[1]; i++)
	{
		dirLightsOut.diffuseColor[i] = dirLights.diffuseColor[i];
		dirLightsOut.specularColor[i] = dirLights.specularColor[i];
		dirLightsOut.direction[i] = vec4(TBN * (dirLights.direction[i].xyz),0.0);
		dirLightsOut.intensity[i] = dirLights.intensity[i];
		dirLightsOut.misc = dirLights.misc;
	}


}