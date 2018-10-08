#include "Collision.hpp"

bool TestCollision(AABB first, AABB second) {
//Xl=.x
//Xr=.x+.w
	bool collidesOnX = (first.x+first.w) >= second.x && first.x <= (second.x+second.w);
	bool collidesOnY = (first.y+first.h) >= second.y && first.y <= (second.y+second.h);
	bool collidesOnZ = (first.z+first.l) >= second.z && first.z <= (second.z+second.l);
	return collidesOnX && collidesOnY && collidesOnZ;
}
