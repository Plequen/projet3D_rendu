
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
#include "Material.h"

class Vertex;
class Object;
class Scene;

class RayTracer {
public:
	static RayTracer * getInstance ();
	static void destroyInstance ();

	typedef enum {None = 0, Uniform = 1, Stochastic = 2} AntialiasingMode;
	typedef enum {NoShadows = 0, Hard = 1, Soft = 2} ShadowsMode;
	typedef enum {AODisabled = 0, AOEnabled = 1, AOOnly = 2} AmbientOcclusionMode;
	typedef enum {MDisabled = 0, MEnabled = 1} MirrorsMode;
	typedef enum {PTDisabled = 0, PTEnabled = 1} PTMode;
	typedef enum {DOFDisabled = 0 , DOFEnabled = 2} DOFMode;

	inline const Vec3Df & getBackgroundColor () const { return backgroundColor;}
	inline void setBackgroundColor (const Vec3Df & c) { backgroundColor = c; }
	void setAAMode(int m) { antialiasingMode = static_cast<AntialiasingMode>(m); }
	void setAAGrid(unsigned int grid) { aaGrid = grid; }
	void setShadowsMode(int m) { shadowsMode = static_cast<ShadowsMode>(m); }
	void setAOMode(int m) { ambientOcclusionMode = static_cast<AmbientOcclusionMode>(m); }
	void setRaysAO(int r) { raysAO = r; } 
	void setPercentageAO(float p) { percentageAO = p; } 
	void setConeAO(float c) { coneAO = c; } 
	void setIntensityAO(float i) { intensityAO = i; } 
	void setLightDiscretization(int d) { nbPointsDisc = d; } 
	void setMirrorsMode(int m) { mirrorsMode = static_cast<MirrorsMode>(m); }
	void setNbMaxReflexion(int m) { nbMaxReflexion = m; } 
	void setRaysPT(int r) { raysPT = r; } 
	void setIterationsPT(int i) { iterationsPT = i; } 
	void setPTMode(int m) { ptMode = static_cast<PTMode>(m); }
	void setfocusBlurSamples( int s) { focusBlurSamples = s;}
	void setfocalDistance(double f) {focalDistance = f;}
	void setaperture(double f){aperture = f;}
	void setdofMode(int s) {dofMode = static_cast<DOFMode>(s);}
	
	void rayTrace(const Vec3Df& origin, Vec3Df& dir, unsigned& nbReflexion,
				  std::vector<unsigned>& objectsIntersected,
				  std::vector<Vertex>& verticesIntersected,
				  std::vector<Vec3Df>& directionsIntersected);
	Vec3Df rayTrace(const Vec3Df& origin, Vec3Df& dir, float& visibility);

	Vec3Df computeColor(const Vertex& intersectedVertex, const Object& o, const Vec3Df& dir, float& visibility, float& occlusionRate);
	float computeOcclusionRate(const Vec3Df& intersectedPoint, const Vec3Df& normal);
	float computeShadowVisibility(const Vec3Df& intersectedPoint, const Object& o);

	Vec3Df computeFinalColor(const std::vector<unsigned>& objectsIntersected,
				    const std::vector<float>& visibilitiesIntersected,
				    const std::vector<float>& occlusionRatesIntersected,
				    const std::vector<Vec3Df>& colorsIntersected);

	
	void gaussianFilter(std::vector<std::vector<float> >& visibility, const float SIGMA, const unsigned int sizeMask, unsigned int screenWidth, unsigned int screenHeight);
	Vec3Df pathTrace(const Vec3Df& origin, Vec3Df& dir, unsigned int iterations, bool alreadyDiffused); 
    
	QImage render (const Vec3Df & camPos,
		const Vec3Df & viewDirection,
		const Vec3Df & upVector,
		const Vec3Df & rightVector,
		float fieldOfView,
		float aspectRatio,
		unsigned int screenWidth,
		unsigned int screenHeight);
    
protected:
	inline RayTracer() : antialiasingMode(None), shadowsMode(NoShadows), ambientOcclusionMode(AODisabled), mirrorsMode(MDisabled), ptMode(PTDisabled), aaGrid(1), raysAO(10), percentageAO(0.05f), coneAO(180.f), intensityAO(1.f), nbPointsDisc(50), raysPT(10), iterationsPT(0) , focusBlurSamples(1), focalDistance(5.0) , aperture(5.6) ,dofMode(DOFDisabled){}
	inline virtual ~RayTracer () {}
    
private:
	Vec3Df backgroundColor;
	AntialiasingMode antialiasingMode;
	ShadowsMode shadowsMode;
	AmbientOcclusionMode ambientOcclusionMode;
	MirrorsMode mirrorsMode;
	bool transparencyMode;
	PTMode ptMode;
	unsigned int aaGrid;
	unsigned int raysAO;
	float percentageAO;
	float coneAO;
	float intensityAO;
	int nbMaxReflexion;
	unsigned int nbPointsDisc; // nb of points on the area light source (discretization)
	unsigned int raysPT;
	unsigned int iterationsPT;
	DOFMode dofMode;
	unsigned int focusBlurSamples;
	double focalDistance ;
	double aperture;
};


#endif // RAYTRACER_H

// Some Emacs-Hints -- please don't remove:
//
//  Local Variables:
//  mode:C++
//  tab-width:4
//  End:
