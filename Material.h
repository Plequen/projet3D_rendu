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
	inline Material () : diffuse (0.8f), specular (0.2f), color (0.5f, 0.5f, 0.5f), mirror(false)  {}
	inline Material (float diffuse, float specular, const Vec3Df & color, bool mirror)
	: diffuse (diffuse), specular (specular), color (color), mirror(mirror) {}
	inline Material (float diffuse, float specular, const Vec3Df & color)
	: diffuse (diffuse), specular (specular), color (color), mirror(false) {}
	virtual ~Material () {}

	inline float getDiffuse () const { return diffuse; }
	inline float getSpecular () const { return specular; }
	inline Vec3Df getColor () const { return color; }
	bool isMirror() const { return mirror; }

	inline void setDiffuse (float d) { diffuse = d; }
	inline void setSpecular (float s) { specular = s; }
	inline void setColor (const Vec3Df & c) { color = c; }

	Vec3Df computeColor(Vec3Df& normal, Vec3Df directionIn, Vec3Df colorIn, Vec3Df directionOut) const;
    
private:
	float diffuse;
	float specular;
	Vec3Df color;
	bool mirror;
};


#endif // MATERIAL_H

// Some Emacs-Hints -- please don't remove:
//
//  Local Variables:
//  mode:C++
//  tab-width:4
//  End:
