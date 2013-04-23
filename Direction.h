
#ifndef DIRECTION_H
#define DIRECTION_H

#include <cstdlib>
#include <vector>
#include "Vec3D.h"

class Direction {
public:
	Direction();

	static void computeBase(const Vec3Df& normal, Vec3Df& base1, Vec3Df& base2);
	static Vec3Df random(const Vec3Df& normal, const Vec3Df& base1, const Vec3Df& base2, float cone);
	static Vec3Df random(const Vec3Df& normal, float cone);

private:
};


#endif
