
#ifndef AREALIGHT_H
#define AREALIGHT_H

#include "Light.h"
#include "Ray.h"
#include <cstdlib>
#include <vector>

class AreaLight : public Light {
public:
	inline AreaLight() : Light(), rayon(1.0f), orientation(-1.0f, -1.0f, -1.0f) {orientation.normalize(); discretize(10); }
	inline AreaLight(const Vec3Df & pos, const Vec3Df & color, float intensity, float r, const Vec3Df& o) : Light(pos, color, intensity), rayon(r), orientation(o) { orientation.normalize(); discretize(10); }
	virtual ~AreaLight() {}

	inline const Vec3Df& getOrientation() const { return orientation; }
	inline const std::vector<Vec3Df>& getDiscretization() const { return discretization; }
	inline float getRayon() const { return rayon; }
	void discretize(unsigned k);

	bool intersects(Ray& ray, float& distance) const;

private:
	std::vector<Vec3Df> discretization;
	float rayon;
	Vec3Df orientation;
};


#endif
