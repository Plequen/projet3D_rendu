#include <cstdlib>
#include <vector>
#include "Direction.h"

#define EPISLON	0.0001f

Direction::Direction() {}

// do not normalize
void Direction::computeBase(const Vec3Df& normal, Vec3Df& base1, Vec3Df& base2) {
	Vec3Df u1(1.f, 0.f, 0.f);
	base1 = Vec3Df::crossProduct(normal, u1);
	if (base1.getLength() == 0.f) {
		base1 = Vec3Df(0.f, 1.f, 0.f);
		base2 = Vec3Df(0.f, 0.f, 1.f);
	}
	else
		base2 = Vec3Df::crossProduct(normal, base1);
}

Vec3Df Direction::random(const Vec3Df& normal, const Vec3Df& base1, const Vec3Df& base2, float cone) {
	// computes a random direction
	float rdm = (float) rand() / ((float) RAND_MAX);
	float rdm2 = (float) rand() / ((float) RAND_MAX);
	Vec3Df tmp(1.f, M_PI * rdm * (cone / 180.f) / 2.f, rdm2 * 2 * M_PI);
	Vec3Df aux = Vec3Df::polarToCartesian(tmp);
	// puts the direction in the hemisphere supported by the normal of the point
	tmp[0] = base1[0] * aux[0] + base2[0] * aux[1] + normal[0] * aux[2];
	tmp[1] = base1[1] * aux[0] + base2[1] * aux[1] + normal[1] * aux[2];
	tmp[2] = base1[2] * aux[0] + base2[2] * aux[1] + normal[2] * aux[2];
	tmp.normalize();
	
	return tmp;
}

Vec3Df Direction::random(const Vec3Df& normal, float cone) {
	Vec3Df base1, base2;
	Direction::computeBase(normal, base1, base2);
	return random(normal, base1, base2, cone);
}



