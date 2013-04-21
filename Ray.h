// *********************************************************
// Ray Class
// Author : Tamy Boubekeur (boubek@gmail.com).
// Copyright (C) 2010 Tamy Boubekeur.
// All rights reserved.
// *********************************************************

#ifndef RAY_H
#define RAY_H

#include <iostream>
#include <vector>
#include <stack>

#include "Vec3D.h"
#include "BoundingBox.h"
#include "Object.h"
#include "Light.h"
#include "Scene.h"
#include "BRDF.h"

const float epsilon = 0.0001f;
const int NOMBRE_MAX_RECURSION = 2;

class Ray {
public:
    inline Ray () {}
    inline Ray (const Vec3Df & origin, const Vec3Df & direction)
        : origin (origin), direction (direction) {}
    inline virtual ~Ray () {}

    inline const Vec3Df & getOrigin () const { return origin; }
    inline Vec3Df & getOrigin () { return origin; }
    inline const Vec3Df & getDirection () const { return direction; }
    inline Vec3Df & getDirection () { return direction; }
    inline void setOrigin(Vec3Df& _origin) { origin = _origin;}
    inline void setDirection(Vec3Df& _direction) {direction = _direction;}

    bool intersect (const BoundingBox & bbox, Vec3Df & intersectionPoint) const;
    bool intersect (const BoundingBox & bbox, float & t) const;
    bool intersectObject(const Object & o, Vec3Df & intersectionPoint, Vec3Df & normal) const;
    bool intersectTriangle(const Vec3Df & va, const Vec3Df & vb, const Vec3Df & vc, float & t, float & coef1, float & coef2) const;
    float computeShadowRays(const Vec3Df& point, const Vec3Df& normal, const std::vector<Object> objects, const Light& lights); 
    Vec3Df mirrorReflection(const Object& object, const Vec3Df& intersectionPoint, const Vec3Df& normal, const Scene* scene, const Vec3Df& direction, const Vec3Df& camPos, int nombreRecursion);
    
private:
    Vec3Df origin;
    Vec3Df direction;
};


#endif // RAY_H

// Some Emacs-Hints -- please don't remove:
//
//  Local Variables:
//  mode:C++
//  tab-width:4
//  End:
