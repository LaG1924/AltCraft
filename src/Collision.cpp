#include "Collision.hpp"

bool TestCollision(const AABB& first, const AABB& second) {
	double firstXl = first.x;
	double firstXr = first.x + first.w;

	double firstYl = first.y;
	double firstYr = first.y + first.h;

	double firstZl = first.z;
	double firstZr = first.z + first.l;


	double secondXl = second.x;
	double secondXr = second.x + second.w;

	double secondYl = second.y;
	double secondYr = second.y + second.h;

	double secondZl = second.z;
	double secondZr = second.z + second.l;

	bool collidesOnX = firstXr >= secondXl && firstXl <= secondXr;
	bool collidesOnY = firstYr >= secondYl && firstYl <= secondYr;
	bool collidesOnZ = firstZr >= secondZl && firstZl <= secondZr;

	return collidesOnX && collidesOnY && collidesOnZ;
}
