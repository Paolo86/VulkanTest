
#include "Light.h"

Light::Light()
{
	diffuseColor = glm::vec3(1, 1, 1);
	specularColor = glm::vec3(1, 1, 1);
	intensity = 1.0f;


}
Light::~Light()
{

}

void Light::GetDiffuseColor(float& r, float& g, float& b)
{
	r = diffuseColor.x;
	g = diffuseColor.y;
	b = diffuseColor.z;
}
void Light::GetSpecularColor(float& r, float& g, float& b)
{
	r = specularColor.x;
	g = specularColor.y;
	b = specularColor.z;
}

void Light::SetDiffuseColor(float r, float g, float b)
{
	diffuseColor.x = r;
	diffuseColor.y = g;
	diffuseColor.z = b;
}
void Light::SetSpecularColor(float r, float g, float b)
{
	specularColor.x = r;
	specularColor.y = g;
	specularColor.z = b;
}