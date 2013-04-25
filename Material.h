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
		inline Material () : diffuse (0.8f), specular (0.2f), color (0.5f, 0.5f, 0.5f), glossiness(0.0f), reflectivity(0.0f), colorBlendingFactor(0.0f), refraction(1.0f), transparency(0.0f) {}
		inline Material (float diffuse, float specular, const Vec3Df & color, float glossiness, float reflectivity, float colorBlendingFactor, float refraction, float transparency) 
			: diffuse (diffuse), specular (specular), color (color), glossiness(glossiness), reflectivity(reflectivity), colorBlendingFactor(colorBlendingFactor), refraction(refraction), transparency(transparency) {}
		inline Material (float diffuse, float specular, const Vec3Df & color)
			: diffuse (diffuse), specular (specular), color (color), glossiness(0.0f), reflectivity(0.0f), colorBlendingFactor(0.0f), refraction(1.0f), transparency(0.0f) {}
		virtual ~Material () {}

		Vec3Df computeColor(const Vec3Df& normal, const Vec3Df& directionIn, const Vec3Df& colorIn, const Vec3Df& directionOut) const;
		inline float getDiffuse () const { return diffuse; }
		inline float getSpecular () const { return specular; }
		inline Vec3Df getColor () const { return color; }
		inline float getGlossiness () const { return glossiness; }
		inline float getReflectivity () const { return reflectivity; }
		inline float getColorBlendingFactor () const { return colorBlendingFactor; }
		inline float getRefraction () const { return refraction; }
		inline float getTransparency () const { return transparency; }

		inline void setDiffuse (float d) { diffuse = d; }
		inline void setSpecular (float s) { specular = s; }
		inline void setColor (const Vec3Df & c) { color = c; }

	private:
		float diffuse;
		float specular;
		Vec3Df color;

		//mirror parameters
		float glossiness;    // angle in degree of the cone around the reflexion direction where the reflexion direction can be randomly deviated
		float reflectivity;  // between 0 (no reflection) and 1 (perfect mirror)
		float colorBlendingFactor; // between 0 and 1, weigthed means of the color of the mirror and the color of the reflected object

		//glass parameters
		float refraction; // refraction indices, > 1
		float transparency; // between O (not transparent) and 1 (fully transparent)

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
