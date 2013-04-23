
#include "AreaLight.h"

void AreaLight::discretize(unsigned k) {
	if (discretization.size() != k)
		discretization.resize(k);	
	Vec3Df x1(0.0f, 0.0f, 0.0f);
	Vec3Df y1;
	if (orientation[0] <= 0.00001f && orientation[0] >= -0.00001f)
		x1 = Vec3Df(0.0f, -orientation[2], orientation[1]);
	else
		x1 = Vec3Df(- orientation[2] / orientation[1], 1.0f, 0.0f);
	y1 = Vec3Df::crossProduct(x1, -orientation);
	for (unsigned i = 0 ; i < k ; i++) {
		float r = rayon * rand() / RAND_MAX;
		float angle = 2 * 3.14 * rand() / RAND_MAX;
		Vec3Df v(r * cos(angle), r * sin(angle), 0.0f);
		Vec3Df v2(x1[0] * v[0] + y1[0] * v[1], x1[1] * v[0] + y1[1] * v[1], x1[2] * v[0] + y1[2] * v[1]);
		discretization[i] = pos + v2;
	}
}

bool AreaLight::intersects(Ray& ray, float& distance) const {
	// the ray does not "intersect" the light if it can't reveive light from it
	if (Vec3Df::dotProduct(ray.getDirection(), orientation) > 0.f)
		return false;

	// get the equation of the plane supporting the light
	float d = Vec3Df::dotProduct(-orientation, pos);
	// distance between the origin of the ray and the intersection point
	float t = - (d + Vec3Df::dotProduct(orientation, ray.getOrigin())) / Vec3Df::dotProduct(orientation, ray.getDirection());
	if (t <= 0.0001f) // not a valid intersection
		return false;
	if (distance < t) // not in the range
		return false;
	if (Vec3Df::distance(ray.getOrigin()+t*ray.getDirection(), pos) > rayon) // not on the light
		return false;

	distance = t;
	return true;
} 
