#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

struct _ViewProjection {

	glm::mat4 projection;
	glm::mat4 view;

};

typedef struct {
	glm::mat4 model;
} UboModel;