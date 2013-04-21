
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

#define INFINITE_DISTANCE	1000000.0f		
#define EPSILON	0.00001f

static RayTracer * instance = NULL;

RayTracer * RayTracer::getInstance () {
	if (instance == NULL)
		instance = new RayTracer ();
	return instance;
}

void RayTracer::destroyInstance () {
	if (instance != NULL) {
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

	// variable parameters
	unsigned nbPointsDisc = 100; // nb of points on the area light source (discretization)

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
					if (hasIntersection) {
						// if object o is a mirror
						// then we do all the operations (shadows, phong shading, ambient occlusion)
						// on the reflected point
						if(scene->getObjects()[intersectedObject].getMaterial().isMirror())
						{
							const Object& auxO = scene->getObjects()[intersectedObject];
							float distReflexion=INFINITE_DISTANCE;
							Vertex intersectionReflexion;
							unsigned int leafIdReflexion;
							Vec3Df reflectionDir = 2*Vec3Df::dotProduct(intersectedVertex.getNormal(), -dir)
								*intersectedVertex.getNormal() + dir;
							for (unsigned int n = 0 ; n < scene->getObjects().size() ; n++) {
								const Object& ob = scene->getObjects()[n];
								Ray reflexionRay(intersectedVertex.getPos() + auxO.getTrans() - ob.getTrans(),reflectionDir);
								if (ob.intersectsRay(reflexionRay, intersectionReflexion,
											distReflexion, leafIdReflexion)) {

									dir=-(camPos-auxO.getTrans()-(intersectionReflexion.getPos()+ob.getTrans()));
									dir.normalize();
									intersectedObject=n;
									intersectedVertex=intersectionReflexion;
								}
							}
						}

						const Object& o = scene->getObjects()[intersectedObject];
						const Material& material = o.getMaterial();
						// intersected point in the object reference : intersectedPoint
						// intersected point in the world reference : intersectedPoint + o.getTrans()
						// let's put the intersected point in the world reference
						const Vec3Df& intersectedPoint = intersectedVertex.getPos() + o.getTrans();
						Vec3Df normal = intersectedVertex.getNormal();
						normal.normalize();
						// color the pixel following the leaf it belongs to in the kdtree
					//	c = Vec3Df(intersectedLeafId % 3 == 0 ? 1.0f : 0.0f, intersectedLeafId % 3 == 1 ? 1.0f : 0.0f, intersectedLeafId % 3 == 2 ? 1.0f : 0.0f);
						if (ambientOcclusionMode != AODisabled) {
							float sphereRadius = percentageAO * scene->getBoundingBox().getSize();
							Vertex intersectionOcclusion;
							unsigned int leafIdOcclusion;
							unsigned int occlusions = 0;
							Vec3Df base1(0, -normal[2], normal[1]);
							Vec3Df base2 = Vec3Df::crossProduct(normal, base1);
							Vec3Df rayDirection;
							for (unsigned int k = 0 ; k < raysAO ; k++) {
								// computes a random direction for the ray
								float rdm = (float) rand() / ((float) RAND_MAX);
								float rdm2 = (float) rand() / ((float) RAND_MAX);
								Vec3Df tmp (1.f, M_PI * rdm * (coneAO / 180.f) / 2.f, rdm2 * 2 * M_PI);
								Vec3Df aux = Vec3Df::polarToCartesian(tmp);
								// places the direction in the hemisphere supported by the normal of the point
								rayDirection[0] = base1[0] * aux[0] + base2[0] * aux[1] + normal[0] * aux[2];
								rayDirection[1] = base1[1] * aux[0] + base2[1] * aux[1] + normal[1] * aux[2];
								rayDirection[2] = base1[2] * aux[0] + base2[2] * aux[1] + normal[2] * aux[2];
								float distOcclusion = sphereRadius; 
								for (unsigned int n = 0 ; n < scene->getObjects().size() ; n++) {
									const Object& ob = scene->getObjects()[n];
									Ray occlusionRay(intersectedPoint - ob.getTrans(), rayDirection);
									if (ob.intersectsRay(occlusionRay, intersectionOcclusion, distOcclusion, leafIdOcclusion)) {
										occlusions++;
										break;
									}
								}
							}
							float occlusionRate = 1.f - ((float) occlusions) / raysAO;
							c += intensityAO * Vec3Df(occlusionRate, occlusionRate, occlusionRate);
						}
						if (ambientOcclusionMode != AOOnly) {
							float distShadow;
							Vertex intersectionShadow;
							unsigned int leafIdShadow;
							//vector<Light>& sceneLights = scene->getLights();
							vector<AreaLight>& sceneLights = scene->getAreaLights();
							//
							// Phong Shading
							//
							for (unsigned int k = 0 ; k < sceneLights.size() ; k++) {
								Vec3Df lightDirection = sceneLights[k].getPos() - intersectedPoint;
								lightDirection.normalize();

								// shadows
								// visibility is between O and 1, 0 if not visible, 1 is completely visible
								float visibility = (float) nbPointsDisc;

								if (shadowsMode == Hard) {
									distShadow = INFINITE_DISTANCE;
									bool shadowFound = false;
									for (unsigned int n = 0 ; n < scene->getObjects().size() ; n++) {
										const Object& ob = scene->getObjects()[n];
										Ray shadowRay(intersectedPoint - ob.getTrans(), lightDirection);
										if (ob.intersectsRay(shadowRay, intersectionShadow, distShadow, leafIdShadow)) {
											visibility = 0.0f;
											break;
										}
									}
								}
								else if (shadowsMode == Soft) {
									// random set of points on the area light source
									sceneLights[k].discretize(nbPointsDisc);
									const vector<Vec3Df>& discretization = sceneLights[k].getDiscretization();

									// cast one ray for each discret point of the area light source
									for(unsigned p=0; p<nbPointsDisc; p++)
									{
										distShadow = INFINITE_DISTANCE;
										Vec3Df lightPosDisc=discretization[p];
										Vec3Df directionToLightDisc = lightPosDisc - (intersectedPoint+o.getTrans());
										directionToLightDisc.normalize();


										for (unsigned int n = 0; n < scene->getObjects().size (); n++) 
										{
											const Object& ob = scene->getObjects()[n];
											Ray shadowRay(intersectedPoint + o.getTrans() - ob.getTrans(), directionToLightDisc);
											if (ob.intersectsRay(shadowRay, intersectionShadow, distShadow, leafIdShadow)) 
											{
												visibility--;
												break;
											}
										}
									}
								}
								visibility /= (float) nbPointsDisc;

								if (visibility < EPSILON)
									continue;

								float diff = Vec3Df::dotProduct(normal, lightDirection);
								Vec3Df r = 2 * diff * normal - lightDirection;
								if (diff <= EPSILON)
									diff = 0.f;
								r.normalize();
								float spec = Vec3Df::dotProduct(r, -dir); 
								if (spec <= EPSILON)
									spec = 0.f;
								c += visibility * sceneLights[k].getIntensity() * (material.getDiffuse() * diff + material.getSpecular() * spec) * sceneLights[k].getColor() * material.getColor();
							}
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
