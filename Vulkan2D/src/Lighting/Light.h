#pragma once

#include <glm/glm.hpp>

/**
* @class Light
* @brief Generic class for a light object
*
* @author Paolo Ferri
* @version 01
* @date 15/03/2019
*
*
* @bug No known bugs.
*/
class Light 
{
public:
	friend class LightManager;

	enum LightType
	{
		POINT,
		DIRECTIONAL
	};

	/**
	* @brief					Create a new light object
	*
	* @post						The light object is created
	* @param lightName			The light object name
	* @post type				The light object type
	*/
	Light();

	/**
	* @brief					Destroy the light object
	* @pre						The light object must exist
	* @post						The light object is destroyed
	*/
	virtual ~Light();

	/**
	* @brief					Returns the diffuse color of the light
	* @pre						The light object must exist
	* @post						The light object diffuse color is returned
	* @param r					The red channel amount
	* @param g					The green channel amount
	* @param b					The blue channel amount
	*/
	void GetDiffuseColor(float& r, float& g, float& b);

	/**
	* @brief					Returns the specular color of the light
	* @pre						The light object must exist
	* @post						The light object specular color is returned
	* @param r					The red channel amount
	* @param g					The green channel amount
	* @param b					The blue channel amount
	*/
	void GetSpecularColor(float& r, float& g, float& b);

	/**
	* @brief					Returns the diffuse color of the light
	* @pre						The light object must exist
	* @post						The light object diffuse color is returned
	* @return					The light object diffuse color
	*/
	glm::vec3& GetDiffuseColor() { return diffuseColor; }

	/**
	* @brief					Returns the specular color of the light
	* @pre						The light object must exist
	* @post						The light object specular color is returned
	* @return					The light object specular color
	*/
	glm::vec3& GetSpecularColor() { return specularColor; }

	/**
	* @brief					Sets the diffuse the color of the light
	* @pre						The light object must exist
	* @post						The light object diffuse color is changed
	* @param r					The red channel amount
	* @param g					The green channel amount
	* @param b					The blue channel amount
	*/
	void SetDiffuseColor(float r, float g, float b);

	/**
	* @brief					Sets the specular the color of the light
	* @pre						The light object must exist
	* @post						The light object specular color is changed
	* @param r					The red channel amount
	* @param g					The green channel amount
	* @param b					The blue channel amount
	*/
	void SetSpecularColor(float r, float g, float b);


	/**
	* @brief					Get the light intensity
	* @pre						The light object must exist
	* @post						The light object intensity is returned
	* @return					The light intensity
	*/
	float& GetIntensity() { return intensity; }

	/**
	* @brief					Set the light intensity
	* @pre						The light object must exist
	* @post						The light object intensity is set
	* @param i					The new intensity value
	*/
	void SetIntensity(float i) { intensity = i; }



protected:

	/**
	* @brief					The light intensity
	*/
	float intensity;

	/**
	* @brief					The light diffuse color
	*/
	glm::vec3 diffuseColor;

	/**
	* @brief					The light specular color
	*/
	glm::vec3 specularColor;

};



