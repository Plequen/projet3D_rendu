// *********************************************************
// Material Class
// Author : Tamy Boubekeur (boubek@gmail.com).
// Copyright (C) 2010 Tamy Boubekeur.
// All rights reserved.
// *********************************************************

#include "Material.h"
#define EPSILON	0.00001f

Vec3Df Material::computeColor(const Vec3Df& normal, const Vec3Df& directionIn, const Vec3Df& colorIn, const Vec3Df& directionOut) const {
	float diff = Vec3Df::dotProduct(normal, -directionIn);
	Vec3Df r = 2 * diff * normal + directionIn;
	if (diff <= EPSILON)
		diff = 0.f;
	r.normalize();
	float spec = Vec3Df::dotProduct(r, directionOut); 
	if (spec <= EPSILON)
		spec = 0.f;
	spec = 0.f;
	return (this->diffuse * diff + this->specular * spec) * colorIn * this->color;
}

