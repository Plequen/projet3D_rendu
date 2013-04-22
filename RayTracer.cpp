
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

Vec3Df RayTracer::rayTrace(const Vec3Df& origin, Vec3Df& dir, unsigned int iterations) {
	if (iterations == 0)
		return Vec3Df(0, 0, 0);

	Scene* scene = Scene::getInstance();

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

	Vec3Df finalColor(0.0f,0.0f,0.0f);
	// compute the color to give to the current pixel
	if (hasIntersection) {
		// if object o is a mirror
		// then we do all the operations (shadows, phong shading, ambient occlusion)
		// on the reflected point

		vector<Vec3Df> colorsIntersected;
		vector<Vertex> listVertexIntersected;
		listVertexIntersected.push_back(intersectedVertex);
		vector<float> listVisibilitiesIntersected;
		vector<float> listOcclusionRatesIntersected;
		vector<unsigned int> listObjectsIntersected;
		listObjectsIntersected.push_back(intersectedObject);
		vector<Vec3Df> listDirectionIntersected;
		listDirectionIntersected.push_back(dir);

		if (mirrorsMode == MEnabled) 
		{
			unsigned nbReflexion=0;
			while(  scene->getObjects()[intersectedObject].getMaterial().getReflexionCoef()>0.0f && nbReflexion < nbMaxReflexion)
			{
				const Object& auxO = scene->getObjects()[intersectedObject];
				float distReflexion=INFINITE_DISTANCE;
				Vertex intersectionReflexion;
				Vertex intersectionReflexionTemp=intersectedVertex;
				unsigned int leafIdReflexion;
				Vec3Df reflectionDir = 2*Vec3Df::dotProduct(intersectedVertex.getNormal(), -dir)
					*intersectedVertex.getNormal() + dir;
				if(auxO.getMaterial().getGlossiness()>0.0f)
				{
					Vec3Df base1(0, -reflectionDir[2], reflectionDir[1]);
					Vec3Df base2 = Vec3Df::crossProduct(reflectionDir, base1);
					// computes a random direction for the ray
					float rdm = (float) rand() / ((float) RAND_MAX);
					float rdm2 = (float) rand() / ((float) RAND_MAX);
					Vec3Df tmp (1.f, M_PI * rdm * ((auxO.getMaterial().getGlossiness()) / 180.f) / 2.f, rdm2 * 2 * M_PI);
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
				listVertexIntersected.push_back(intersectedVertex);
				listObjectsIntersected.push_back(intersectedObject);
				listDirectionIntersected.push_back(dir);
			}
		}

		for(unsigned i=0; i<listVertexIntersected.size(); i++)
		{
			intersectedVertex=listVertexIntersected[i];
			intersectedObject=listObjectsIntersected[i];
			dir=listDirectionIntersected[i];
			const Object& o = scene->getObjects()[intersectedObject];
			
			float visibility = (float) nbPointsDisc;
			float occlusionRate = 1.f;
			
			colorsIntersected.push_back(computeColor(scene, intersectedVertex, o, dir, iterations, visibility, occlusionRate));

			listVisibilitiesIntersected.push_back(visibility);
			listOcclusionRatesIntersected.push_back(occlusionRate);

			//Point is not visible, no need do compute the color of the next reflected point
			if(visibility<0.001f || occlusionRate < 0.001f)
			{
				for(unsigned j=i+1; j<listVertexIntersected.size(); j++)
				{
					listVisibilitiesIntersected.push_back(0.0f);
					listOcclusionRatesIntersected.push_back(0.0f);
					colorsIntersected.push_back(Vec3Df(0.0f,0.0f,0.0f));
				}
				break;
			}
		}

		if (ambientOcclusionMode == AOOnly)
		{
			finalColor = Vec3Df(listOcclusionRatesIntersected[0],listOcclusionRatesIntersected[0],listOcclusionRatesIntersected[0]);
		}
		else
		{
			finalColor=colorsIntersected[colorsIntersected.size()-1];
			for(unsigned i=0; i<listVertexIntersected.size(); i++)
				finalColor*=listVisibilitiesIntersected[i]*listOcclusionRatesIntersected[i];
		}
	}

	return finalColor;	
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

					c += rayTrace(camPos, dir, iterationsPT);
				}
			}
			c *= 255.0f / (raysPerPixel * raysPerPixel);
			image.setPixel(i, j, qRgb(clamp(c[0], 0, 255), clamp(c[1], 0, 255), clamp(c[2], 0, 255)));
		}
	}
	progressDialog.setValue (100);
	return image;
}

Vec3Df RayTracer::computeColor(Scene* scene, const Vertex& intersectedVertex, const Object& o, const Vec3Df& dir, unsigned iterations, float& visibility, float& occlusionRate)
{
	Vec3Df c(0.f, 0.f, 0.f);

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
		occlusionRate = intensityAO * (1.f - ((float) occlusions) / raysAO);
		//c += intensityAO * Vec3Df(occlusionRate, occlusionRate, occlusionRate);
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

			float diff = Vec3Df::dotProduct(normal, lightDirection);
			Vec3Df r = 2 * diff * normal - lightDirection;
			if (diff <= EPSILON)
				diff = 0.f;
			r.normalize();
			float spec = Vec3Df::dotProduct(r, -dir); 
			if (spec <= EPSILON)
				spec = 0.f;
			c += /*visibility * */sceneLights[k].getIntensity() * (material.getDiffuse() * diff + material.getSpecular() * spec) * sceneLights[k].getColor() * material.getColor();
		}
	}

	visibility /= (float) nbPointsDisc;

	// path-tracing : recursive ray-tracing
	if (iterations > 1) {
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

			c += (1.f / (raysPT)) * rayTrace(intersectedPoint, rayDirection, iterations - 1);
		}
	}

	return c;

}
