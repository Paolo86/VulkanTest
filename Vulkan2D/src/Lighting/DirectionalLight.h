#pragma once
#include "Light.h"

/**
* @class DirectionalLight
* @brief Specialized class for a directional light object
*
* @author Paolo Ferri
* @version 01
* @date 15/03/2019
*
*
* @bug No known bugs.
*/
class DirectionalLight : public Light
{
public:
	/**
	* @brief						Create a new directional light object
	*
	* @post							The light object is created
	* @param castsShadows			Whether the light will cast shadows
	*/
	DirectionalLight();
	/**
	* @brief					Destroy the light object
	* @pre						The light object must exist
	* @post						The light object is destroyed
	*/
	~DirectionalLight();

	glm::vec3 direction;

private:


};
