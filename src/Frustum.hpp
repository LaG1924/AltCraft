#pragma once

#include <glm/glm.hpp>

class Frustum {
    enum FrustumSide {
        RIGHT,
        LEFT,
        BOTTOM,
        TOP,
		FAR,
        NEAR,
		SIDE_COUNT,
    };

	glm::vec4 planes[SIDE_COUNT];

public:    
	Frustum(const glm::mat4 &vpMat);

	~Frustum() = default;

	inline static float GetDistanceToPoint(const glm::vec4 &plane, const glm::vec3 &pos) {
		return plane.x * pos.x + plane.y * pos.y + plane.z * pos.z + plane.w;
	}

	bool TestPoint(const glm::vec3 &pos);

	bool TestSphere(const glm::vec3 &pos, float radius);
};