#ifndef BRDF_H
#define BRDF_H

#include <vector>
#include <iostream>
#include <cmath>
#include "Light.h"
#include "Vec3D.h"
#include "Ray.h"
#include "Object.h"
#include "Scene.h"
#include "Material.h"

Vec3Df phong(const Vec3Df& point, const Vec3Df& normal, const Object& object,const Scene* scene, const Vec3Df& direction, const Vec3Df& camPos);

#endif

