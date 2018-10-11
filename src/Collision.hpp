#pragma once

inline bool TestCollisionV(double stX, double stA,double enX,double enA) {
	return stX+stA>=enX && stX<=enX+enA;
}
