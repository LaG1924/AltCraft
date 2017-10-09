#include "Frustum.hpp"

#include <glm/gtc/type_ptr.hpp>

void Frustum::NormalizePlane(FrustumSide side) {
    float magnitude = (float)sqrt(frustum[side][A] * frustum[side][A] + frustum[side][B] * frustum[side][B] + frustum[side][C] * frustum[side][C]);

    frustum[side][A] /= magnitude;
    frustum[side][B] /= magnitude;
    frustum[side][C] /= magnitude;
    frustum[side][D] /= magnitude;
}

void Frustum::UpdateFrustum(const glm::mat4& vpmat) {
    vp = vpmat;
    float *clip = glm::value_ptr(vp);

    frustum[RIGHT][A] = clip[3] - clip[0];
    frustum[RIGHT][B] = clip[7] - clip[4];
    frustum[RIGHT][C] = clip[11] - clip[8];
    frustum[RIGHT][D] = clip[15] - clip[12];
    NormalizePlane(RIGHT);


    frustum[LEFT][A] = clip[3] + clip[0];
    frustum[LEFT][B] = clip[7] + clip[4];
    frustum[LEFT][C] = clip[11] + clip[8];
    frustum[LEFT][D] = clip[15] + clip[12];
    NormalizePlane(LEFT);

    frustum[BOTTOM][A] = clip[3] + clip[1];
    frustum[BOTTOM][B] = clip[7] + clip[5];
    frustum[BOTTOM][C] = clip[11] + clip[9];
    frustum[BOTTOM][D] = clip[15] + clip[13];
    NormalizePlane(BOTTOM);

    frustum[TOP][A] = clip[3] - clip[1];
    frustum[TOP][B] = clip[7] - clip[5];
    frustum[TOP][C] = clip[11] - clip[9];
    frustum[TOP][D] = clip[15] - clip[13];
    NormalizePlane(TOP);

    frustum[BACK][A] = clip[3] - clip[2];
    frustum[BACK][B] = clip[7] - clip[6];
    frustum[BACK][C] = clip[11] - clip[10];
    frustum[BACK][D] = clip[15] - clip[14];
    NormalizePlane(BACK);

    frustum[FRONT][A] = clip[3] + clip[2];
    frustum[FRONT][B] = clip[7] + clip[6];
    frustum[FRONT][C] = clip[11] + clip[10];
    frustum[FRONT][D] = clip[15] + clip[14];
    NormalizePlane(FRONT);
}

bool Frustum::TestPoint(VectorF point) {
    for (int i = 0; i < 6; i++) {
        if (frustum[i][A] * point.x + frustum[i][B] * point.y + frustum[i][C] * point.z + frustum[i][D] <= 0) {
            return false;
        }
    }
    return true;
}