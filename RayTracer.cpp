// *********************************************************
// Ray Tracer Class
// Author : Tamy Boubekeur (boubek@gmail.com).
// Copyright (C) 2012 Tamy Boubekeur.
// All rights reserved.
// *********************************************************

#include "RayTracer.h"
#include "Ray.h"
#include "Scene.h"
#include "KDTree.h"
#include "BRDF.h"
#include <QProgressDialog>

using namespace std;

void computeShadowRays(const vector<Object>& objects, const vector<Light>& lights, vector<bool>& shadows);
static RayTracer * instance = NULL;

RayTracer * RayTracer::getInstance () 
{
	if (instance == NULL)
		instance = new RayTracer ();
	return instance;
}

void RayTracer::destroyInstance () 
{
	if (instance != NULL) 
	{
		delete instance;
		instance = NULL;
	}
}

inline int clamp (float f, int inf, int sup) 
{
	int v = static_cast<int> (f);
	return (v < inf ? inf : (v > sup ? sup : v));
}

QImage RayTracer::render (const Vec3Df & camPos,
		const Vec3Df & direction,
		const Vec3Df & upVector,
		const Vec3Df & rightVector,
		float fieldOfView,
		float aspectRatio,
		unsigned int screenWidth,
		unsigned int screenHeight) 
{
	QImage image (QSize (screenWidth, screenHeight), QImage::Format_RGB888);
	Scene * scene = Scene::getInstance ();
	const BoundingBox & bbox = scene->getBoundingBox ();
	const Vec3Df & minBb = bbox.getMin ();
	const Vec3Df & maxBb = bbox.getMax ();
	const Vec3Df rangeBb = maxBb - minBb;
	QProgressDialog progressDialog ("Raytracing...", "Cancel", 0, 100);
	progressDialog.show ();
	for (unsigned int i = 0; i < screenWidth; i++) 
	{
		progressDialog.setValue ((100*i)/screenWidth);
		for (unsigned int j = 0; j < screenHeight; j++) 
		{
			float tanX = tan (fieldOfView)*aspectRatio;
			float tanY = tan (fieldOfView);
			Vec3Df stepX = (float (i) - screenWidth/2.f)/screenWidth * tanX * rightVector;
			Vec3Df stepY = (float (j) - screenHeight/2.f)/screenHeight * tanY * upVector;
			Vec3Df step = stepX + stepY;
			Vec3Df dir = direction + step;
			dir.normalize ();
			Vec3Df intersectionPoint;
			Vec3Df normal;
			float smallestIntersectionDistance = 1000000.f;
			Vec3Df c (backgroundColor);
			for (unsigned int k = 0; k < scene->getObjects().size (); k++) 
			{
				const Object & o = scene->getObjects()[k];
				const Material& mat = o.getMaterial();
				Ray ray (camPos-o.getTrans (), dir);
				bool hasIntersection = ray.intersectObject (o, intersectionPoint, normal);
				if (hasIntersection) 
				{	
					float intersectionDistance = Vec3Df::squaredDistance (intersectionPoint + o.getTrans (), camPos);
					if (intersectionDistance < smallestIntersectionDistance) 
					{
						c = Vec3Df(0.0f,0.0f,0.0f);
						Material material = o.getMaterial();
						normal.normalize();
						vector<Light> sceneLights = scene->getLights();

						//*** Phong shading with shawdows ***/
						c += phong(intersectionPoint, normal, o, scene, -dir ,camPos);
						if(mat.getShininess()>0.0001f)//Ajout de l'effet mirroir
						{
							c += ray.mirrorReflection(o, intersectionPoint,normal, scene, -dir, camPos, 0);
						}
						c=255.0f*c;
						smallestIntersectionDistance = intersectionDistance;
					}
				}
			}
			image.setPixel (i, j, qRgb (clamp (c[0], 0, 255), clamp (c[1], 0, 255), clamp (c[2], 0, 255)));
		}
	}
	progressDialog.setValue (100);
	return image;
}




