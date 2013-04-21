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
#include <QProgressDialog>

//#define	ANTIALIASING	1
#define INFINITE_DISTANCE	1000000.0f		

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

QImage RayTracer::render(const Vec3Df& camPos,
		const Vec3Df& direction,
		const Vec3Df& upVector,
		const Vec3Df& rightVector,
		float fieldOfView,
		float aspectRatio,
		unsigned int screenWidth,
		unsigned int screenHeight) {
	QImage image (QSize (screenWidth, screenHeight), QImage::Format_RGB888);
	Scene * scene = Scene::getInstance ();
	const BoundingBox & bbox = scene->getBoundingBox ();
	const Vec3Df & minBb = bbox.getMin ();
	const Vec3Df & maxBb = bbox.getMax ();
	const Vec3Df rangeBb = maxBb - minBb;
	QProgressDialog progressDialog ("Raytracing...", "Cancel", 0, 100);
	progressDialog.show ();
	unsigned int raysPerPixel = antialiasingMode == Uniform ? aaGrid : 1;
	for (unsigned int i = 0; i < screenWidth; i++) {
		progressDialog.setValue ((100*i)/screenWidth);
		for (unsigned int j = 0; j < screenHeight; j++) {
			float tanX = tan (fieldOfView) * aspectRatio;
			float tanY = tan (fieldOfView);
			float stepPixelX = float (i) - screenWidth / 2.f;
			float stepPixelY = float (j) - screenHeight / 2.f;
			Vec3Df c = Vec3Df(0.0f, 0.0f, 0.0f);
			for (unsigned int l = 0 ; l < raysPerPixel ; l++) {
				for (unsigned int m = 0 ; m < raysPerPixel ; m++) {
					float stepAAX = (0.5f + float (l)) / raysPerPixel - 0.5f;  
					float stepAAY = (0.5f + float (m)) / raysPerPixel - 0.5f;  
					Vec3Df stepX = (stepPixelX + stepAAX) / screenWidth * tanX * rightVector;
					Vec3Df stepY = (stepPixelY + stepAAY) / screenHeight * tanY * upVector;
					Vec3Df step = stepX + stepY;
					Vec3Df dir = direction + step;
					dir.normalize();

					bool hasIntersection = false;
					Vertex intersection;
					unsigned int leafId;
					unsigned int intersectedObject;
					Vertex intersectedVertex;
					unsigned int intersectedLeafId;
					// search the nearest intersection point between the ray and the scene
					float smallestIntersectionDistance = INFINITE_DISTANCE;
					for (unsigned int k = 0 ; k < scene->getObjects().size() ; k++) {
						const Object& o = scene->getObjects()[k];
						Ray ray(camPos - o.getTrans(), dir);
						// tests if the ray intersects the object and if the distance from the camera is the lowest so far
						if (o.intersectsRay(ray, intersection, smallestIntersectionDistance, leafId)) {
							hasIntersection = true;	
							intersectedVertex = intersection;
							intersectedObject = k;
							intersectedLeafId = leafId;
						}
					}
					// compute the color to give to the current pixel
					float distShadow;
					Vertex intersectionShadow;
					unsigned int leafIdShadow;
					if (hasIntersection) {
						const Object& o = scene->getObjects()[intersectedObject];
						const Material& material = o.getMaterial();
						const Vec3Df& intersectedPoint = intersectedVertex.getPos();
						Vec3Df normal = intersectedVertex.getNormal();
						normal.normalize();
						vector<Light>& sceneLights = scene->getLights();
						// color the pixel following the leaf it belongs to in the kdtree
					//	c = Vec3Df(intersectedLeafId % 3 == 0 ? 1.0f : 0.0f, intersectedLeafId % 3 == 1 ? 1.0f : 0.0f, intersectedLeafId % 3 == 2 ? 1.0f : 0.0f);
						//
						// Phong Shading
						//
						for (unsigned int k = 0 ; k < sceneLights.size() ; k++) {
							// intersected point in the world reference : intersectedPoint + o.getTrans()
							// light position in the world reference : sceneLights[k].getPos()
							// intersected point in the object reference : intersectedPoint
							// light position in the object reference : sceneLights[k].getPos() - o.getTrans()
							Vec3Df lightDirection = sceneLights[k].getPos() - (intersectedPoint + o.getTrans());
							lightDirection.normalize();

							// shadows
							distShadow = INFINITE_DISTANCE;
							bool shadowFound = false;
							for (unsigned int n = 0 ; n < scene->getObjects().size() ; n++) {
								const Object& ob = scene->getObjects()[n];
								Ray shadowRay(intersectedPoint + o.getTrans() - ob.getTrans(), lightDirection);
								if (ob.intersectsRay(shadowRay, intersectionShadow, distShadow, leafIdShadow)) {
									shadowFound = true;
									break;
								}
							}
							if (shadowFound)
								continue;

							float diff = Vec3Df::dotProduct(normal, lightDirection);
							Vec3Df r = 2 * diff * normal - lightDirection;
							if (diff <= 0.00001f)
								diff = 0.00001f;
							r.normalize();
							Vec3Df v = - (intersectedPoint + o.getTrans());
							v.normalize();
							float spec = Vec3Df::dotProduct(r, -dir); 
							if (spec <= 0.00001f)
								spec = 0.00001f;
							c += sceneLights[k].getIntensity() * (material.getDiffuse() * diff + material.getSpecular() * spec) * sceneLights[k].getColor() * material.getColor();
						}
					}
				}
			}
			c *= 255.0f / (raysPerPixel * raysPerPixel);
			image.setPixel(i, j, qRgb(clamp(c[0], 0, 255), clamp(c[1], 0, 255), clamp(c[2], 0, 255)));
		}
	}
	progressDialog.setValue (100);
	return image;
}
