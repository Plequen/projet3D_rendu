// *********************************************************
// Ray Tracer Class
// Author : Tamy Boubekeur (boubek@gmail.com).
// Copyright (C) 2012 Tamy Boubekeur.
// All rights reserved.
// *********************************************************

#ifndef RAYTRACER_H
#define RAYTRACER_H

#include <iostream>
#include <vector>
#include <QImage>

#include "Vec3D.h"

class RayTracer {
public:
    static RayTracer * getInstance ();
    static void destroyInstance ();

	typedef enum {None = 0, Uniform = 1, Poisson = 2} AntialiasingMode;
	typedef enum {NoShadows = 0, Hard = 1, Soft = 2, AmbientOcclusion = 3} ShadowsMode;

    inline const Vec3Df & getBackgroundColor () const { return backgroundColor;}
    inline void setBackgroundColor (const Vec3Df & c) { backgroundColor = c; }
	void setAAMode(int m) { antialiasingMode = static_cast<AntialiasingMode>(m); }
	void setAAGrid(unsigned int grid) { aaGrid = grid; }
	void setShadowsMode(int m) { shadowsMode = static_cast<ShadowsMode>(m); }
    
    QImage render (const Vec3Df & camPos,
                   const Vec3Df & viewDirection,
                   const Vec3Df & upVector,
                   const Vec3Df & rightVector,
                   float fieldOfView,
                   float aspectRatio,
                   unsigned int screenWidth,
                   unsigned int screenHeight);
    
protected:
    inline RayTracer() : antialiasingMode(None), shadowsMode(NoShadows), aaGrid(1) {}
    inline virtual ~RayTracer () {}
    
private:
    Vec3Df backgroundColor;
	AntialiasingMode antialiasingMode;
	ShadowsMode shadowsMode;
	unsigned int aaGrid;
};


#endif // RAYTRACER_H

// Some Emacs-Hints -- please don't remove:
//
//  Local Variables:
//  mode:C++
//  tab-width:4
//  End:
