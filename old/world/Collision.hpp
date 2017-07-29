#pragma once

struct AABB {
	double x,y,z;
	double w,l,h;
};

bool TestCollision(AABB first, AABB second);