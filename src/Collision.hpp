#pragma once
//#define TestCollisionV(stX,stA,enX,enA) (stX+stA) >= enX && stX <= (enX+enA)
/*struct AABB {
	double x,y,z;
	double w,l,h;
};*/
/*struct AABBV{
//	double a;
//	double x;
    double a,x;
};*/
//bool TestCollision(AABB first, AABB second);
// For example: stX=AABB.x; stA=AABB.w; // st=start; en=end;
bool TestCollisionV(double stX, double stA,double enX,double enA);
