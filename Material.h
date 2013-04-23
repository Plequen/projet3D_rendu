// *********************************************************
// Material Class
// Author : Tamy Boubekeur (boubek@gmail.com).
// Copyright (C) 2010 Tamy Boubekeur.
// All rights reserved.
// *********************************************************

#ifndef MATERIAL_H
#define MATERIAL_H

#include <iostream>
#include <vector>

#include "Vec3D.h"

// Ce modèle suppose une couleur spéculaire blanche (1.0, 1.0, 1.0)

class Material {
public:
    inline Material () : diffuse (0.8f), specular (0.2f), color (0.5f, 0.5f, 0.5f), glossiness(0.0f), reflectivity(0.0f), mirrorColorBlendingFactor(0.0f), refraction(1.0f), transparency(0.0f) {}
    inline Material (float diffuse, float specular, const Vec3Df & color, float glossiness, float reflectivity, float mirrorColorBlendingFactor, float refraction, float transparency) 
        : diffuse (diffuse), specular (specular), color (color), glossiness(glossiness), reflectivity(reflectivity), mirrorColorBlendingFactor(mirrorColorBlendingFactor), refraction(refraction), transparency(transparency) {}
    inline Material (float diffuse, float specular, const Vec3Df & color)
        : diffuse (diffuse), specular (specular), color (color), glossiness(0.0f), reflectivity(0.0f), mirrorColorBlendingFactor(0.0f), refraction(1.0f), transparency(0.0f) {}
    virtual ~Material () {}

    Vec3Df computeColor(Vec3Df& normal, Vec3Df directionIn, Vec3Df colorIn, Vec3Df directionOut) const;
    inline float getDiffuse () const { return diffuse; }
    inline float getSpecular () const { return specular; }
    inline Vec3Df getColor () const { return color; }
    inline float getGlossiness () const { return glossiness; }
    inline float getReflectivity () const { return reflectivity; }
    inline float getMirrorColorBlendingFactor () const { return mirrorColorBlendingFactor; }
    inline float getRefraction () const { return refraction; }
    inline float getTransparency () const { return transparency; }

    inline void setDiffuse (float d) { diffuse = d; }
    inline void setSpecular (float s) { specular = s; }
    inline void setColor (const Vec3Df & c) { color = c; }
    
private:
    float diffuse;
    float specular;
    Vec3Df color;
    float glossiness;
    float reflectivity;
    float mirrorColorBlendingFactor;
    float refraction;
    float transparency;

	// path tracing
	float reflexion;
	float transmission;
};


#endif // MATERIAL_H

// Some Emacs-Hints -- please don't remove:
//
//  Local Variables:
//  mode:C++
//  tab-width:4
//  End:
