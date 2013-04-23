
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
#define NB_MAX_REFLEXION 10

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

Vec3Df RayTracer::pathTrace(const Vec3Df& origin, Vec3Df& dir, unsigned int iterations) {
	bool hasIntersection = false;
	Vertex intersection;
	unsigned int leafId;
	unsigned int intersectedObject;
	Vertex intersectedVertex;
	unsigned int intersectedLeafId;
	Scene* scene = Scene::getInstance();

	// search the nearest intersection point between the ray and the scene
	vector<Object>& sceneObjects = scene->getObjects();
	float smallestIntersectionDistance = INFINITE_DISTANCE;
	for (unsigned int i = 0 ; i < sceneObjects.size() ; i++) {
		const Object& o = sceneObjects[i];
		Ray ray(origin - o.getTrans(), dir);
		// tests if the ray intersects the object and if the distance from the origin is the lowest so far
		if (o.intersectsRay(ray, intersection, smallestIntersectionDistance, leafId)) {
			hasIntersection = true;	
			intersectedVertex = intersection;
			intersectedObject = i;
			intersectedLeafId = leafId;
		}
	}
	// search now for an intersection with a light
	vector<AreaLight>& sceneLights = scene->getAreaLights();
	bool intersectionIsLight = false;
	unsigned int intersectedLight;
	for (unsigned int i = 0 ; i < sceneLights.size() ; i++) {
		const AreaLight& l = sceneLights[i];
		Ray ray(origin, dir);
		// tests if the ray intersects the light and if the distance from the origin is the lowest so far
		if (l.intersects(ray, smallestIntersectionDistance)) {
			hasIntersection = true;	
			intersectionIsLight = true;
			intersectedLight = i;
		}
	}

	if (!hasIntersection) // the ray doesn't intersect an object nor a light
		return Vec3Df(0.f, 0.f, 0.f);

	if (intersectionIsLight) // the intersected point is on a light
		return sceneLights[intersectedLight].getIntensity() * sceneLights[intersectedLight].getColor();
	
	// Otherwise, the intersected point is on an object
	const Object& o = scene->getObjects()[intersectedObject];
	const Material& material = o.getMaterial();
	const Vec3Df& intersectedPoint = intersectedVertex.getPos() + o.getTrans();
	Vec3Df normal = intersectedVertex.getNormal();
	normal.normalize();

	Vec3Df c(0.f, 0.f, 0.f);
	// path-tracing : recursive ray-tracing
	if (iterations > 0) {
		Vec3Df base1(0, -normal[2], normal[1]);
		Vec3Df base2 = Vec3Df::crossProduct(normal, base1);
		Vec3Df rayDirection;
		for (unsigned int i = 0 ; i < raysPT ; i++) {
			// computes a random direction for the ray
			float rdm = (float) rand() / ((float) RAND_MAX);
			float rdm2 = (float) rand() / ((float) RAND_MAX);
			Vec3Df tmp (1.f, M_PI * rdm * (180.f / 180.f) / 2.f, rdm2 * 2 * M_PI);
			Vec3Df aux = Vec3Df::polarToCartesian(tmp);
			// places the direction in the hemisphere supported by the normal of the point
			rayDirection[0] = base1[0] * aux[0] + base2[0] * aux[1] + normal[0] * aux[2];
			rayDirection[1] = base1[1] * aux[0] + base2[1] * aux[1] + normal[1] * aux[2];
			rayDirection[2] = base1[2] * aux[0] + base2[2] * aux[1] + normal[2] * aux[2];
			rayDirection.normalize();

			c += (1.f / (raysPT)) * material.computeColor(normal, -rayDirection, pathTrace(intersectedPoint, rayDirection, iterations - 1), -dir);
		}
	}
	return c;
}

Vec3Df RayTracer::rayTrace(const Vec3Df& origin, Vec3Df& dir) {
	Scene* scene = Scene::getInstance();

	bool glossy=true;

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
		Ray ray(origin - o.getTrans(), dir);
		// tests if the ray intersects the object and if the distance from the camera is the lowest so far
		if (o.intersectsRay(ray, intersection, smallestIntersectionDistance, leafId)) {
			hasIntersection = true;	
			intersectedVertex = intersection;
			intersectedObject = k;
			intersectedLeafId = leafId;
		}
	}

	// compute the color to give to the current pixel
	Vec3Df c(0.f, 0.f, 0.f);
	if (hasIntersection) {
		// if object o is a mirror
		// then we do all the operations (shadows, phong shading, ambient occlusion)
		// on the reflected point
		if (mirrorsMode == MEnabled) 
		{
			unsigned nbReflexion=0;
			while(  scene->getObjects()[intersectedObject].getMaterial().isMirror() && nbReflexion < NB_MAX_REFLEXION)
			{
				const Object& auxO = scene->getObjects()[intersectedObject];
				float distReflexion=INFINITE_DISTANCE;
				Vertex intersectionReflexion;
				Vertex intersectionReflexionTemp=intersectedVertex;
				unsigned int leafIdReflexion;
				Vec3Df reflectionDir = 2*Vec3Df::dotProduct(intersectedVertex.getNormal(), -dir)
					*intersectedVertex.getNormal() + dir;
				if(glossy)
				{
					float angleSolide=0.0f;
					Vec3Df base1(0, -reflectionDir[2], reflectionDir[1]);
					Vec3Df base2 = Vec3Df::crossProduct(reflectionDir, base1);
					// computes a random direction for the ray
					float rdm = (float) rand() / ((float) RAND_MAX);
					float rdm2 = (float) rand() / ((float) RAND_MAX);
					Vec3Df tmp (1.f, M_PI * rdm * (angleSolide / 180.f) / 2.f, rdm2 * 2 * M_PI);
					Vec3Df aux = Vec3Df::polarToCartesian(tmp);
					// places the direction in the hemisphere supported by the normal of the point
					reflectionDir[0] = base1[0] * aux[0] + base2[0] * aux[1] + reflectionDir[0] * aux[2];
					reflectionDir[1] = base1[1] * aux[0] + base2[1] * aux[1] + reflectionDir[1] * aux[2];
					reflectionDir[2] = base1[2] * aux[0] + base2[2] * aux[1] + reflectionDir[2] * aux[2];
				}
				for (unsigned int n = 0 ; n < scene->getObjects().size() ; n++) {
					const Object& ob = scene->getObjects()[n];
					Ray reflexionRay(intersectedVertex.getPos() + auxO.getTrans() - ob.getTrans(),reflectionDir);
					if (ob.intersectsRay(reflexionRay, intersectionReflexion,
								distReflexion, leafIdReflexion)) {

						dir=-(intersectedVertex.getPos()+auxO.getTrans()-(intersectionReflexion.getPos()+ob.getTrans()));
						intersectedObject=n;
						intersectionReflexionTemp=intersectionReflexion;
					}
				}
				intersectedVertex=intersectionReflexionTemp;
				dir.normalize();
				nbReflexion++;
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
		float occlusionRate = 1.f;
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
			occlusionRate = intensityAO * (1.f - ((float) occlusions) / raysAO);
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
		if (ambientOcclusionMode == AOOnly)
			c += Vec3Df(occlusionRate, occlusionRate, occlusionRate);
		else
			c *= occlusionRate;
	}
	return c;	
}

Vec3Df RayTracer::computeColor(const Material& material, Vec3Df& normal, Vec3Df directionIn, Vec3Df colorIn, Vec3Df directionOut) {
	float diff = Vec3Df::dotProduct(normal, -directionIn);
	Vec3Df r = 2 * diff * normal + directionIn;
	if (diff <= EPSILON)
		diff = 0.f;
	r.normalize();
	float spec = Vec3Df::dotProduct(r, directionOut); 
	if (spec <= EPSILON)
		spec = 0.f;
	return (material.getDiffuse() * diff + material.getSpecular() * spec) * colorIn * material.getColor();
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

					if (ptMode == PTDisabled)
						c += rayTrace(camPos, dir);
					else
						c += pathTrace(camPos, dir, iterationsPT);
				}
			}
			c *= 255.0f / (raysPerPixel * raysPerPixel);
			image.setPixel(i, j, qRgb(clamp(c[0], 0, 255), clamp(c[1], 0, 255), clamp(c[2], 0, 255)));
		}
	}
	progressDialog.setValue (100);
	return image;
}
