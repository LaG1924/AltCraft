#ifndef _FRUSTUM_H
#define _FRUSTUM_H


#include <cmath>
#include <algorithm>
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

class Frustum {
public:
	Frustum() = default;

	~Frustum() = default;

	void CalculateFrustum(glm::mat4 &view_matrix, glm::mat4 &proj_matrix);

	glm::vec4 frustum_planes[6];

	bool TestInsideFrustrum(glm::vec4 Min, glm::vec4 Max);
};

#endif