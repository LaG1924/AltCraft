#include "Frustum.hpp"

Frustum::Frustum(const glm::mat4 &vpMat) {
	planes[RIGHT] = glm::vec4(
		vpMat[0][3] - vpMat[0][0],
		vpMat[1][3] - vpMat[1][0],
		vpMat[2][3] - vpMat[2][0],
		vpMat[3][3] - vpMat[3][0]);

	planes[LEFT] = glm::vec4(
		vpMat[0][3] + vpMat[0][0],
		vpMat[1][3] + vpMat[1][0],
		vpMat[2][3] + vpMat[2][0],
		vpMat[3][3] + vpMat[3][0]);

	planes[BOTTOM] = glm::vec4(
		vpMat[0][3] + vpMat[0][1],
		vpMat[1][3] + vpMat[1][1],
		vpMat[2][3] + vpMat[2][1],
		vpMat[3][3] + vpMat[3][1]);

	planes[TOP] = glm::vec4(
		vpMat[0][3] - vpMat[0][1],
		vpMat[1][3] - vpMat[1][1],
		vpMat[2][3] - vpMat[2][1],
		vpMat[3][3] - vpMat[3][1]);

	planes[FAR] = glm::vec4(
		vpMat[0][3] - vpMat[0][2],
		vpMat[1][3] - vpMat[1][2],
		vpMat[2][3] - vpMat[2][2],
		vpMat[3][3] - vpMat[3][2]);

	planes[NEAR] = glm::vec4(
		vpMat[0][3] + vpMat[0][2],
		vpMat[1][3] + vpMat[1][2],
		vpMat[2][3] + vpMat[2][2],
		vpMat[3][3] + vpMat[3][2]);

	for (auto &plane : planes) {
		float magnitude = sqrt(plane.x * plane.x + plane.y * plane.y + plane.z * plane.z);
		plane.x /= magnitude;
		plane.y /= magnitude;
		plane.z /= magnitude;
		plane.w /= magnitude;
	}
}

bool Frustum::TestPoint(const glm::vec3 &pos) {
	for (const auto &plane : planes) {
		if (GetDistanceToPoint(plane, pos) < 0)
			return false;
	}
	return true;
}

bool Frustum::TestSphere(const glm::vec3 &pos, float radius) {
	for (const auto &plane : planes) {
		if (GetDistanceToPoint(plane, pos) < -radius)			
			return false;
	}
	return true;
}
