#pragma once

struct AABB {
	double x,y,z;
	double w,l,h;
};

bool TestCollision(const AABB& first, const AABB& second);