
#include "AreaLight.h"

void AreaLight::init() {
	orientation.normalize();
	Direction::computeBase(orientation, base1, base2);
	// get the equation of the plane supporting the light
	d = Vec3Df::dotProduct(-orientation, pos);

	discretize(10);
}

void AreaLight::discretize(unsigned k) {
	if (discretization.size() != k)
		discretization.resize(k);	

	for (unsigned i = 0 ; i < k ; i++) {
		float r = rayon * rand() / RAND_MAX;
		float angle = 2 * 3.14 * rand() / RAND_MAX;
		Vec3Df v(r * cos(angle), r * sin(angle), 0.0f);
		Vec3Df v2(base1[0] * v[0] + base2[0] * v[1], base1[1] * v[0] + base2[1] * v[1], base1[2] * v[0] + base2[2] * v[1]);
		discretization[i] = pos + v2;
	}
}
void AreaLight::discretize(unsigned k, const Vec3Df& dir) {
	Direction::computeBase(dir, base1, base2);
	if (discretization.size() != k)
		discretization.resize(k);	

	for (unsigned i = 0 ; i < k ; i++) {
		float r = rayon * rand() / RAND_MAX;
		float angle = 2 * 3.14 * rand() / RAND_MAX;
		Vec3Df v(r * cos(angle), r * sin(angle), 0.0f);
		Vec3Df v2(base1[0] * v[0] + base2[0] * v[1], base1[1] * v[0] + base2[1] * v[1], base1[2] * v[0] + base2[2] * v[1]);
		discretization[i] = pos + v2;
	}
}

Vec3Df AreaLight::randomPoint() const {
	float r = rayon * rand() / RAND_MAX;
	float angle = 2 * M_PI * rand() / RAND_MAX;
	Vec3Df v(r * cos(angle), r * sin(angle), 0.0f);

	return pos + Vec3Df(base1[0] * v[0] + base2[0] * v[1], base1[1] * v[0] + base2[1] * v[1], base1[2] * v[0] + base2[2] * v[1]);
}

bool AreaLight::intersects(Ray& ray, float& distance) const {
	// the ray does not "intersect" the light if it can't receive light from it
	if (Vec3Df::dotProduct(ray.getDirection(), orientation) > 0.f)
		return false;
	
/*	float temp = Vec3Df::crossProduct(ray.getDirection(), -orientation).getLength();

	if (temp > 0.1f)
		return false;*/

	// distance between the origin of the ray and the intersection point
	float t = - (d + Vec3Df::dotProduct(orientation, ray.getOrigin())) / Vec3Df::dotProduct(orientation, ray.getDirection());
	if (t <= 0.0001f) // not a valid intersection
		return false;
	if (distance < t) // not in the range
		return false;
	if (Vec3Df::distance(ray.getOrigin()+t*ray.getDirection(), pos) > rayon) // not on the light
		return false;

	//distance = t;
	return true;
} 
