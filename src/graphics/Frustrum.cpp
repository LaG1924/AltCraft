#include "Frustrum.hpp"

enum FrustumSide {
	RIGHT = 0,        // The RIGHT side of the frustum
	LEFT = 1,        // The LEFT	 side of the frustum
	BOTTOM = 2,        // The BOTTOM side of the frustum
	TOP = 3,        // The TOP side of the frustum
	BACK = 4,        // The BACK	side of the frustum
	FRONT = 5            // The FRONT side of the frustum
};

enum PlaneData {
	A = 0,                // The X value of the plane's normal
	B = 1,                // The Y value of the plane's normal
	C = 2,                // The Z value of the plane's normal
	D = 3                // The distance the plane is from the origin
};

void normalizePlane(glm::vec4 &frustum_plane) {
	float magnitude = (float) sqrt(frustum_plane[A] * frustum_plane[A] + frustum_plane[B] * frustum_plane[B] +
	                               frustum_plane[C] * frustum_plane[C]);
	frustum_plane[A] /= magnitude;
	frustum_plane[B] /= magnitude;
	frustum_plane[C] /= magnitude;
	frustum_plane[D] /= magnitude;
}

void Frustum::CalculateFrustum(glm::mat4 &view_matrix, glm::mat4 &proj_matrix) {
	float proj[16];
	float modl[16];
	const float *projP = glm::value_ptr(proj_matrix);
	const float *modlP = glm::value_ptr(view_matrix);
	for (int i = 0; i < 16; i++) {
		proj[i]=projP[i];
		modl[i]=modlP[i];
	}

	float clip[16]; //clipping planes

	clip[0] = modl[0] * proj[0] + modl[1] * proj[4] + modl[2] * proj[8] + modl[3] * proj[12];
	clip[1] = modl[0] * proj[1] + modl[1] * proj[5] + modl[2] * proj[9] + modl[3] * proj[13];
	clip[2] = modl[0] * proj[2] + modl[1] * proj[6] + modl[2] * proj[10] + modl[3] * proj[14];
	clip[3] = modl[0] * proj[3] + modl[1] * proj[7] + modl[2] * proj[11] + modl[3] * proj[15];

	clip[4] = modl[4] * proj[0] + modl[5] * proj[4] + modl[6] * proj[8] + modl[7] * proj[12];
	clip[5] = modl[4] * proj[1] + modl[5] * proj[5] + modl[6] * proj[9] + modl[7] * proj[13];
	clip[6] = modl[4] * proj[2] + modl[5] * proj[6] + modl[6] * proj[10] + modl[7] * proj[14];
	clip[7] = modl[4] * proj[3] + modl[5] * proj[7] + modl[6] * proj[11] + modl[7] * proj[15];

	clip[8] = modl[8] * proj[0] + modl[9] * proj[4] + modl[10] * proj[8] + modl[11] * proj[12];
	clip[9] = modl[8] * proj[1] + modl[9] * proj[5] + modl[10] * proj[9] + modl[11] * proj[13];
	clip[10] = modl[8] * proj[2] + modl[9] * proj[6] + modl[10] * proj[10] + modl[11] * proj[14];
	clip[11] = modl[8] * proj[3] + modl[9] * proj[7] + modl[10] * proj[11] + modl[11] * proj[15];

	clip[12] = modl[12] * proj[0] + modl[13] * proj[4] + modl[14] * proj[8] + modl[15] * proj[12];
	clip[13] = modl[12] * proj[1] + modl[13] * proj[5] + modl[14] * proj[9] + modl[15] * proj[13];
	clip[14] = modl[12] * proj[2] + modl[13] * proj[6] + modl[14] * proj[10] + modl[15] * proj[14];
	clip[15] = modl[12] * proj[3] + modl[13] * proj[7] + modl[14] * proj[11] + modl[15] * proj[15];

	frustum_planes[RIGHT][A] = clip[3] - clip[0];
	frustum_planes[RIGHT][B] = clip[7] - clip[4];
	frustum_planes[RIGHT][C] = clip[11] - clip[8];
	frustum_planes[RIGHT][D] = clip[15] - clip[12];
	normalizePlane(frustum_planes[RIGHT]);

	frustum_planes[LEFT][A] = clip[3] + clip[0];
	frustum_planes[LEFT][B] = clip[7] + clip[4];
	frustum_planes[LEFT][C] = clip[11] + clip[8];
	frustum_planes[LEFT][D] = clip[15] + clip[12];
	normalizePlane(frustum_planes[LEFT]);

	frustum_planes[BOTTOM][A] = clip[3] + clip[1];
	frustum_planes[BOTTOM][B] = clip[7] + clip[5];
	frustum_planes[BOTTOM][C] = clip[11] + clip[9];
	frustum_planes[BOTTOM][D] = clip[15] + clip[13];
	normalizePlane(frustum_planes[BOTTOM]);

	frustum_planes[TOP][A] = clip[3] - clip[1];
	frustum_planes[TOP][B] = clip[7] - clip[5];
	frustum_planes[TOP][C] = clip[11] - clip[9];
	frustum_planes[TOP][D] = clip[15] - clip[13];
	normalizePlane(frustum_planes[TOP]);

	frustum_planes[BACK][A] = clip[3] - clip[2];
	frustum_planes[BACK][B] = clip[7] - clip[6];
	frustum_planes[BACK][C] = clip[11] - clip[10];
	frustum_planes[BACK][D] = clip[15] - clip[14];
	normalizePlane(frustum_planes[BACK]);

	frustum_planes[FRONT][A] = clip[3] + clip[2];
	frustum_planes[FRONT][B] = clip[7] + clip[6];
	frustum_planes[FRONT][C] = clip[11] + clip[10];
	frustum_planes[FRONT][D] = clip[15] + clip[14];
	normalizePlane(frustum_planes[FRONT]);
}

bool Frustum::TestInsideFrustrum(glm::vec4 Min, glm::vec4 Max) {
	bool inside = true;
	//test all 6 frustum planes
	for (int i = 0; i < 6; i++) {
		//pick closest point to plane and check if it behind the plane
		//if yes - object outside frustum
		float d = std::max(Min.x * frustum_planes[i].x, Max.x * frustum_planes[i].x)
		          + std::max(Min.y * frustum_planes[i].y, Max.y * frustum_planes[i].y)
		          + std::max(Min.z * frustum_planes[i].z, Max.z * frustum_planes[i].z)
		          + frustum_planes[i].w;
		inside &= d > 0;
		//return false; //with flag works faster
	}
	return inside;
}
