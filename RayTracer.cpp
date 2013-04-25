

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
#include "Direction.h"
#include <QProgressDialog>
# include <omp.h>

#define INFINITE_DISTANCE	1000000.0f		
#define EPSILON	0.00001f
#define NB_MAX_REFLEXION 1

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


Vec3Df RayTracer::pathTrace(const Vec3Df& origin, Vec3Df& dir, unsigned int iterations, bool alreadyDiffused, unsigned int& samples) {
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

	Vec3Df c(0.f, 0.f, 0.f);
	if (!alreadyDiffused && !hasIntersection)
		return c;

	// search now for an intersection with one or several lights
	vector<AreaLight>& sceneLights = scene->getAreaLights();
	Ray ray0(origin, dir);
	for (unsigned int i = 0 ; i < sceneLights.size() ; i++) {
		const AreaLight& l = sceneLights[i];
		// tests if the ray intersects the light and if the distance from the origin is lower than for the nearest object
		if (l.intersects(ray0, smallestIntersectionDistance)) {
			c += l.getIntensity() * l.getColor();
			samples++;
		}
	}

	if (!hasIntersection) // the ray doesn't intersect an object
		return c;

	// we have a true intersected point	
	const Object& o = scene->getObjects()[intersectedObject];
	const Material& material = o.getMaterial();
	const Vec3Df& intersectedPoint = intersectedVertex.getPos() + o.getTrans();
	Vec3Df normal = intersectedVertex.getNormal();
	normal.normalize();
	bool isDiffusing = alreadyDiffused || mirrorsMode == MDisabled || (mirrorsMode == MEnabled && !(material.getReflectivity()>0.001f));

	// path-tracing : recursive ray-tracing
	if (iterations < iterationsPT) {
		Vec3Df base1, base2;
		Direction::computeBase(normal, base1, base2);
		Vec3Df rayDirection;
		unsigned int shadowRays = nbPointsDisc;
		if (mirrorsMode == MEnabled && material.getReflectivity()>0.001f) {
			Vec3Df reflectionDir = 2 * Vec3Df::dotProduct(normal, -dir) * normal + dir;
			float glossiness = material.getGlossiness();
			if (glossiness > 0.f)
				reflectionDir = Direction::random(reflectionDir, glossiness);
			c += 0.5 * pathTrace(intersectedPoint, reflectionDir, iterations + 1, isDiffusing, samples);
		}
		else {
			for (unsigned int i = 0 ; i < raysPT ; i++) {
				// computes a random direction for the ray
				rayDirection = Direction::random(normal, base1, base2, 180.f);	
				//c += 0.5f * material.computeColor(normal, -rayDirection, pathTrace(intersectedPoint, rayDirection, iterations - 1, isDiffusing), -dir);
				c +=  (1.f / raysPT) * material.computeColor(normal, -rayDirection, pathTrace(intersectedPoint, rayDirection, iterations + 1, isDiffusing, samples), -dir);
				//c += (1.f / (raysPT + shadowRays)) * material.computeColor(normal, -rayDirection, pathTrace(intersectedPoint, rayDirection, iterations - 1, isDiffusing), -dir);
			}
			// shadow rays
			for (unsigned int i = 0 ; i < shadowRays ; i++) {
				for (unsigned int j = 0 ; j < sceneLights.size() ; j++) {
					int rdm = rand() % sceneLights.size();
					rdm = j;
					Vec3Df directionToLight = sceneLights[rdm].randomPoint() - intersectedPoint;
					directionToLight.normalize();
					c += (1.f / (shadowRays * sceneLights.size())) * material.computeColor(normal, -directionToLight, pathTrace(intersectedPoint, directionToLight, iterations + 1, isDiffusing, samples), -dir);
					//c += (1.f / (raysPT + shadowRays)) * material.computeColor(normal, -directionToLight, pathTrace(intersectedPoint, directionToLight, iterations + 1, isDiffusing), -dir);
				}
			}
		}
	}
	return c;
}

Vec3Df RayTracer::rayTrace(const Vec3Df& origin, Vec3Df& dir) {
	Scene* scene = Scene::getInstance();

	vector<Vertex> verticesIntersected;
	vector<unsigned int> objectsIntersected;
	vector<Vec3Df> directionsIntersected;

	unsigned nbReflexion=0;

	rayTrace(origin, dir, nbReflexion, objectsIntersected, verticesIntersected, directionsIntersected);

	Vertex intersectedVertex;
	unsigned int intersectedObject;
	Vec3Df directionIntersection;

	vector<Vec3Df> colorsIntersected;
	vector<float> visibilitiesIntersected;
	vector<float> occlusionRatesIntersected;

	for(unsigned i=0; i<verticesIntersected.size(); i++)
	{
		intersectedVertex=verticesIntersected[i];
		intersectedObject=objectsIntersected[i];
		directionIntersection=directionsIntersected[i];
		const Object& o = scene->getObjects()[intersectedObject];

		float visibility = (float) nbPointsDisc;
		float occlusionRate = 1.f;

		colorsIntersected.push_back(computeColor(intersectedVertex, o, directionIntersection, visibility, occlusionRate));

		visibilitiesIntersected.push_back(visibility);
		occlusionRatesIntersected.push_back(occlusionRate);

		//Point is not visible, no need do compute the color of the next reflected point
		if(visibility<0.001f || occlusionRate < 0.001f)
		{
			for(unsigned j=i+1; j<verticesIntersected.size(); j++)
			{
				visibilitiesIntersected.push_back(0.0f);
				occlusionRatesIntersected.push_back(0.0f);
				colorsIntersected.push_back(Vec3Df(0.0f,0.0f,0.0f));
			}
			break;
		}
	}

	Vec3Df color=computeFinalColor(objectsIntersected, visibilitiesIntersected, occlusionRatesIntersected, colorsIntersected);
	return color; 

}

void RayTracer::rayTrace(const Vec3Df& origin, Vec3Df& dir, unsigned& nbReflexion,
		std::vector<unsigned>& objectsIntersected,
		std::vector<Vertex>& verticesIntersected,
		std::vector<Vec3Df>& directionsIntersected) {

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


	if (hasIntersection)
	{
		const Object& auxO = scene->getObjects()[intersectedObject];

		if(auxO.getMaterial().getRefraction()>1.0f)
		{
			float n1;
			float n2;
			Vec3Df normal =intersectedVertex.getNormal();

			//first intersection
			if(verticesIntersected.size()==0)
			{
				n1=1.0f;
				n2=auxO.getMaterial().getRefraction();
			}
			else
			{
				//if the current object is the same as the previous
				if(objectsIntersected[objectsIntersected.size()-1]==intersectedObject)
				{
					n1=auxO.getMaterial().getRefraction();	
					n2=1.0f;
					normal=-normal;
				}
				else
				{
					n1=1.0f;
					n2=auxO.getMaterial().getRefraction();
				}
			}

			float cos1 = Vec3Df::dotProduct(normal, -dir);
			float cos2=1-((n1*n1)/(n2*n2))*(1-cos1*cos1);
			if(cos2>0.0f)
			{
				cos2=sqrt(cos2);
				Vec3Df refractedDir;
				if(cos1>0.0f)
					refractedDir=(n1/n2)*dir+((n1/n2)*cos1-cos2)*normal;
				else
					refractedDir=(n1/n2)*dir+((n1/n2)*cos1+cos2)*normal;

				refractedDir.normalize();
				rayTrace(intersectedVertex.getPos() + auxO.getTrans(),refractedDir, nbReflexion, objectsIntersected, verticesIntersected, directionsIntersected);
			}
			else
			{
				Vec3Df reflectedDir=dir+2*cos1*normal;
				reflectedDir.normalize();
				rayTrace(intersectedVertex.getPos() + auxO.getTrans(),reflectedDir, nbReflexion, objectsIntersected, verticesIntersected, directionsIntersected);
			}
		}
		else
		{ 
			verticesIntersected.push_back(intersectedVertex);
			objectsIntersected.push_back(intersectedObject);
			directionsIntersected.push_back(dir);

			if (mirrorsMode == MEnabled) 
			{
				if(  auxO.getMaterial().getReflectivity()>0.000001f && nbReflexion < nbMaxReflexion)
				{
					nbReflexion++;
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
					reflectionDir.normalize();

					rayTrace(intersectedVertex.getPos() + auxO.getTrans(),reflectionDir, nbReflexion, objectsIntersected, verticesIntersected, directionsIntersected);
				}
			}
		}
	}
}

Vec3Df RayTracer::computeFinalColor(const vector<unsigned>& objectsIntersected,
		const vector<float>& visibilitiesIntersected,
		const vector<float>& occlusionRatesIntersected,
		const vector<Vec3Df>& colorsIntersected)
{
	Scene* scene = Scene::getInstance();

	//If no intersection was found, we put the pixel to background color
	if(colorsIntersected.size()==0)
		return backgroundColor;

	Vec3Df finalColor;
	if (ambientOcclusionMode == AOOnly)
	{
		finalColor = Vec3Df(occlusionRatesIntersected[0],occlusionRatesIntersected[0],occlusionRatesIntersected[0]);
	}
	else
	{
		finalColor=colorsIntersected[colorsIntersected.size()-1];

		for(int i=0; i<colorsIntersected.size(); i++)
		{
			const Material& mat = scene->getObjects()[objectsIntersected[i]].getMaterial();
			if(mat.getReflectivity()>0.001f && mirrorsMode == MEnabled)
				finalColor=mat.getReflectivity()*(mat.getMirrorColorBlendingFactor()*mat.getColor()+(1.0f-mat.getMirrorColorBlendingFactor())*finalColor);
			finalColor=visibilitiesIntersected[i]*occlusionRatesIntersected[i]*finalColor;
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

	/*Vec3Df** imageTemp = new Vec3Df*[screenWidth];
	  for (unsigned int i = 0; i < screenWidth; i++)
	  imageTemp[i] = new Vec3Df[screenHeight];
	//float max = 0.f;*/
	QProgressDialog progressDialog ("Raytracing...", "Cancel", 0, 100);
	progressDialog.show ();
	unsigned int raysPerPixel = antialiasingMode == Uniform ? aaGrid : 1;

//#pragma omp parallel for schedule(dynamic, 1, private(c))
	transparencyMode = true;
	unsigned int maxSamples = 1;
	//unsigned int samples = 0;
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
#pragma omp parallel private(l, m)
{
					float stepAAX = (0.5f + float (l)) / raysPerPixel - 0.5f;  
					float stepAAY = (0.5f + float (m)) / raysPerPixel - 0.5f;  
					Vec3Df stepX = (stepPixelX + stepAAX) / screenWidth * tanX * rightVector;
					Vec3Df stepY = (stepPixelY + stepAAY) / screenHeight * tanY * upVector;
					Vec3Df step = stepX + stepY;
					Vec3Df dir = direction + step;
					dir.normalize();

					if (ptMode == PTDisabled)
						c += rayTrace(camPos, dir);
					else {
						unsigned int samples = 0;
						c += pathTrace(camPos, dir, 0, false, samples);
						//if (samples > maxSamples)
							//maxSamples = samples;
					}
}
				}
			}
			//samples = 1;
			c *= 255.0f / (raysPerPixel * raysPerPixel); //* samples);
			image.setPixel(i, j, qRgb(clamp(c[0], 0, 255), clamp(c[1], 0, 255), clamp(c[2], 0, 255)));
			//imageTemp[i][j] = c;
			/*if (c[0] > max)
			  max = c[0];
			  if (c[1] > max)
			  max = c[1];
			  if (c[3] > max)
			  max = c[3];*/
		}
	}
	//cout << "m" << maxSamples << endl;
	//maxSamples = 1;
	/*for (unsigned int i = 0; i < screenWidth; i++) {
		for (unsigned int j = 0; j < screenHeight; j++) {
			Vec3Df c = imageTemp[i][j] / maxSamples;	
			image.setPixel(i, j, qRgb(clamp(c[0], 0, 255), clamp(c[1], 0, 255), clamp(c[2], 0, 255)));
		}
	}
	for (unsigned int i = 0; i < screenWidth; i++)
		delete [] imageTemp[i];
	delete [] imageTemp;*/

	progressDialog.setValue (100);
	return image;
}

Vec3Df RayTracer::computeColor(const Vertex& intersectedVertex, const Object& o, const Vec3Df& dir, float& visibility, float& occlusionRate)
{
	Scene* scene = Scene::getInstance();
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
						if(ob.getMaterial().getTransparency()<0.0001f)
						{
							visibility=0.0f;
							break;
						}
						else
						{
							visibility=ob.getMaterial().getTransparency()*nbPointsDisc;
						}
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
							if(ob.getMaterial().getTransparency()<0.0001f)
							{
								visibility--;
								break;
							}
							else
							{
								visibility-=(1-ob.getMaterial().getTransparency());
							}
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

	return c;

}
