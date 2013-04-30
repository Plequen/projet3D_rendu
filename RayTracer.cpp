

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
#include <random>

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


Vec3Df RayTracer::pathTrace(const Vec3Df& origin, Vec3Df& dir, unsigned int iterations, bool alreadyDiffused, unsigned int reflections, float n) {
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
	float smallestTemp = INFINITE_DISTANCE;
	Vec3Df transFactor = Vec3Df(1.f, 1.f, 1.f);
	for (unsigned int i = 0 ; i < sceneObjects.size() ; i++) {
		const Object& o = sceneObjects[i];
		Ray ray(origin - o.getTrans(), dir);	
		// tests if the ray intersects the object and if the distance from the origin is the lowest so far
		if (o.intersectsRay(ray, intersection, smallestIntersectionDistance, leafId)) {
			if (o.getMaterial().getRefraction() > 1.f) // if the object is transparent
				transFactor *= o.getMaterial().getTransparency() * o.getMaterial().getColor();
			else
				smallestTemp = smallestIntersectionDistance;
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
		if (l.intersects(ray0, smallestTemp))
			c += transFactor * l.getIntensity() * l.getColor();
	}

	if (!hasIntersection) // the ray doesn't intersect an object
		return c;

	// we have a true intersected point	
	const Object& o = scene->getObjects()[intersectedObject];
	const Material& material = o.getMaterial();
	const Vec3Df& intersectedPoint = intersectedVertex.getPos() + o.getTrans();
	Vec3Df normal = intersectedVertex.getNormal();
	normal.normalize();
	bool isDiffusing = alreadyDiffused || mirrorsMode == MDisabled || (mirrorsMode == MEnabled && !(material.getReflectivity() > 0.001f));

	// path-tracing : recursive ray-tracing
	if (iterations < iterationsPT) {
		Vec3Df base1, base2;
		Direction::computeBase(normal, base1, base2);
		Vec3Df rayDirection;
		unsigned int shadowRays = nbPointsDisc;
		Vec3Df cMirror(0.f, 0.f, 0.f);
		Vec3Df cIndirect(0.f, 0.f, 0.f);
		Vec3Df cDirect(0.f, 0.f, 0.f);
		Vec3Df cRefracted(0.f, 0.f, 0.f);
		if (mirrorsMode == MEnabled && material.getReflectivity() > 0.f && reflections > 0) {
			Vec3Df reflectionDir = 2 * Vec3Df::dotProduct(normal, -dir) * normal + dir;
			float glossiness = material.getGlossiness();
			if (glossiness > 0.f)
				reflectionDir = Direction::random(reflectionDir, glossiness);
			cMirror += pathTrace(intersectedPoint, reflectionDir, iterations, isDiffusing, reflections - 1, n);
		}
		if (material.getRefraction() > 1.f || material.getTransparency() > 0.f) {
			float n1; // first environment (in)
			float n2; // second environment (out)

			float cos1 = Vec3Df::dotProduct(normal, -dir);
			if (cos1 > 0.f) { // entering the object
				n1 = n;
				n2 = material.getRefraction();
			}
			else { // getting out of the object
				n1 = material.getRefraction();
				n2 = 1.f;
			}
			float cos2 = 1 - ((n1 * n1) / (n2 * n2)) * (1 - cos1 * cos1);
			if (cos2 >= 0.f) { // there is indeed refraction
				cos2 = sqrt(cos2);
				Vec3Df refractedDir;
				if (cos1 >= 0.f)		
					refractedDir = (n1 / n2) * dir + ((n1 / n2) * cos1 - cos2) * normal;
				else
					refractedDir = (n1 / n2) * dir + ((n1 / n2) * cos1 + cos2) * normal;
				refractedDir.normalize();
				cRefracted += material.getColor() * pathTrace(intersectedPoint, refractedDir, iterations, isDiffusing, reflections, n2);
			}
			else {
				if (reflections > 0) {
					Vec3Df reflectionDir = dir + 2 * cos1 * normal;
					cRefracted += 1.0 / material.getTransparency() * pathTrace(intersectedPoint, reflectionDir, iterations, isDiffusing, reflections - 1, n);
				}
			}
		}
		if (material.getReflectivity() < 1.f) {
			for (unsigned int i = 0 ; i < raysPT ; i++) {
				// computes a random direction for the ray
				rayDirection = Direction::random(normal, base1, base2, 180.f);	
				cIndirect +=  (1.f / raysPT) * material.computeColor(normal, -rayDirection, pathTrace(intersectedPoint, rayDirection, iterations + 1, isDiffusing, reflections, n), -dir);
			}
			// shadow rays
			for (unsigned int i = 0 ; i < shadowRays ; i++) {
				for (unsigned int j = 0 ; j < sceneLights.size() ; j++) {
					int rdm = rand() % sceneLights.size();
					rdm = j;
					Vec3Df directionToLight = sceneLights[rdm].randomPoint() - intersectedPoint;
					directionToLight.normalize();
					cDirect += (1.f / (shadowRays * sceneLights.size())) * material.computeColor(normal, -directionToLight, pathTrace(intersectedPoint, directionToLight, iterations + 1, isDiffusing, reflections, n), -dir);
				}
			}
		}
		c += ((1.f - (mirrorsMode == MEnabled ? material.getReflectivity() : 0.f)) * (cIndirect + cDirect) + (mirrorsMode == MEnabled ? 1.f : 0.f) * material.getReflectivity() * cMirror + material.getTransparency() * cRefracted);
	}
	return c;
}


void RayTracer::rayTrace(const Vec3Df& origin, Vec3Df& dir, double time, vector<float>& visibility, vector<Vec3Df>& colors) {
	Scene* scene = Scene::getInstance();

	vector<Vertex> verticesIntersected;
	vector<unsigned int> objectsIntersected;
	vector<Vec3Df> directionsIntersected;

	unsigned nbReflexion=0;

	rayTrace(origin, dir, nbReflexion, objectsIntersected, verticesIntersected, directionsIntersected, time);

	Vertex intersectedVertex;
	unsigned int intersectedObject;
	Vec3Df directionIntersection;

	vector<vector<Vec3Df> > colorsIntersected;
	vector<vector<float> > visibilitiesIntersected;
	vector<float> occlusionRatesIntersected;

	vector<Vec3Df> c;
	c.resize(scene->getAreaLights().size());
	vector<float> v;
	v.resize(scene->getAreaLights().size());

	for(unsigned i=0; i<verticesIntersected.size(); i++)
	{
		intersectedVertex=verticesIntersected[i];
		intersectedObject=objectsIntersected[i];
		directionIntersection=directionsIntersected[i];
		const Object& o = scene->getObjects()[intersectedObject];

		float occlusionRate = 1.f;

		computeColor(intersectedVertex, o, directionIntersection, c, v, occlusionRate);

		colorsIntersected.push_back(c);
		visibilitiesIntersected.push_back(v);
		occlusionRatesIntersected.push_back(occlusionRate);

		//Point is not visible, no need do compute the color of the next reflected point
		bool b=true;
		for(unsigned k=0; k < scene->getAreaLights().size(); k++)
			b=b && v[k] < 0.001f;

		if(b || occlusionRate < 0.001f)
		{
			for(unsigned k=0; k<scene->getAreaLights().size(); k++)
			{
				c[k]=Vec3Df(0.0f,0.0f,0.0f);
				v[k]=0.0f;
			}
			for(unsigned j=i+1; j<verticesIntersected.size(); j++)
			{
				visibilitiesIntersected.push_back(v);
				occlusionRatesIntersected.push_back(0.0f);
				colorsIntersected.push_back(c);
			}
			break;
		}
	}

	computeFinalColor(objectsIntersected, visibilitiesIntersected, occlusionRatesIntersected, colorsIntersected, colors);
	for(unsigned k=0; k<scene->getAreaLights().size(); k++)
	{
		visibility[k] = 1.0f;	
		for(unsigned int j = 0 ; j< visibilitiesIntersected.size() ; j++)
			visibility[k] *= visibilitiesIntersected[j][k];
	}
}

void RayTracer::rayTrace(const Vec3Df& origin, Vec3Df& dir, unsigned& nbReflexion,
		std::vector<unsigned>& objectsIntersected,
		std::vector<Vertex>& verticesIntersected,
		std::vector<Vec3Df>& directionsIntersected, double time) {

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
		Object& o = scene->getObjects()[k];
		o.animate(time);
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

			//first intersection, the ray starts from the camera
			if(verticesIntersected.size()==0)
			{
				n1=1.0f;
				n2=auxO.getMaterial().getRefraction();
			}
			else
			{
				//if the current object is the same as the previous, it means the ray is inside the object
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

			//ray is refracted
			if(cos2>0.0f)
			{
				cos2=sqrt(cos2);
				Vec3Df refractedDir;
				if(cos1>0.0f)
					refractedDir=(n1/n2)*dir+((n1/n2)*cos1-cos2)*normal;
				else
					refractedDir=(n1/n2)*dir+((n1/n2)*cos1+cos2)*normal;

				refractedDir.normalize();
				rayTrace(intersectedVertex.getPos() + auxO.getTrans(),refractedDir, nbReflexion, objectsIntersected, verticesIntersected, directionsIntersected, time);
			}

			// ray is reflected
			else
			{
				Vec3Df reflectedDir=dir+2*cos1*normal;
				reflectedDir.normalize();
				rayTrace(intersectedVertex.getPos() + auxO.getTrans(),reflectedDir, nbReflexion, objectsIntersected, verticesIntersected, directionsIntersected, time);
			}
		}

		// intersected object is not transparent
		else
		{ 
			// we add this vertex, the direction from which it was intersected and the object it belongs to the path of the path followed
			verticesIntersected.push_back(intersectedVertex);
			objectsIntersected.push_back(intersectedObject);
			directionsIntersected.push_back(dir);
			/*
			 */
			if (mirrorsMode == MEnabled) 
			{ 
				if(  auxO.getMaterial().getReflectivity()>0.000001f && nbReflexion < nbMaxReflexion)
				{
					nbReflexion++;
					Vec3Df reflectionDir = 2*Vec3Df::dotProduct(intersectedVertex.getNormal(), -dir)
						*intersectedVertex.getNormal() + dir;
					// if mirror is glossy, the direction is deviated randomly in a cone of angle "glossiness"	
					if(auxO.getMaterial().getGlossiness()>0.0f)
						reflectionDir = Direction::random(reflectionDir, auxO.getMaterial().getGlossiness());

					rayTrace(intersectedVertex.getPos() + auxO.getTrans(),reflectionDir, nbReflexion, objectsIntersected, verticesIntersected, directionsIntersected, time);
				}
			}
		}
	}
}

// Given all the colors of the intersected point on the path followed by the rays, 
// computes the color of the pixel
void RayTracer::computeFinalColor(const vector<unsigned>& objectsIntersected,
		const vector<vector<float> >& visibilitiesIntersected,
		const vector<float>& occlusionRatesIntersected,
		const vector<vector<Vec3Df> >& colorsIntersected,
		vector<Vec3Df>& colors)
{
	Scene* scene = Scene::getInstance();

	//If no intersection was found, we put the pixel to background color
	if(colorsIntersected.size()==0)
	{
		for(unsigned k=0; k<scene->getAreaLights().size(); k++)
			colors[k]=backgroundColor;
	}
	else
	{
		// only show ambient occlusion
		if (ambientOcclusionMode == AOOnly)
		{
			for(unsigned k=0; k<scene->getAreaLights().size(); k++)
				colors[k]=Vec3Df(occlusionRatesIntersected[0],occlusionRatesIntersected[0],occlusionRatesIntersected[0]);
		}

		else
		{
			// the last color on the path of the rays
			for(unsigned k=0; k<scene->getAreaLights().size(); k++)
				colors[k]=colorsIntersected[colorsIntersected.size()-1][k];

			for(int i=0; i<colorsIntersected.size(); i++)
			{
				const Material& mat = scene->getObjects()[objectsIntersected[i]].getMaterial();
				if(mat.getReflectivity()>0.001f && mirrorsMode == MEnabled)
				{
					for(unsigned k=0; k<scene->getAreaLights().size(); k++)		
						colors[k]=mat.getReflectivity()*(mat.getColorBlendingFactor()*mat.getColor()+(1.0f-mat.getColorBlendingFactor())*colors[k]);
				}
				//finalColor*=visibilitiesIntersected[i]*occlusionRatesIntersected[i];
				for(unsigned k=0; k<scene->getAreaLights().size(); k++)
					colors[k]*=occlusionRatesIntersected[i];
			}
		}
	}
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
	unsigned int raysPerPixel = (antialiasingMode == Uniform || antialiasingMode == Stochastic)  ? aaGrid : 1;

	//Depth of field Control Parameters and variable definition
	Vec3Df directionOptics = direction, rightVectorOptics = rightVector, upVectorOptics = upVector;
	directionOptics.normalize();
	directionOptics = directionOptics*focalDistance;
	rightVectorOptics.normalize();
	upVectorOptics.normalize();
	rightVectorOptics = rightVectorOptics*focalDistance;
	upVectorOptics = upVectorOptics * focalDistance;
	default_random_engine generator;
	uniform_real_distribution<float> distribution1(0.0,2*M_PI);
	uniform_real_distribution<float> distribution2(0,focalDistance/aperture);
	uniform_real_distribution<float> distribution3(-0.5,0.5);
	//End DoF

	//Motion Blur parameters
	bool mbON = true;
	double current_time = 0;
	double interval_time = 1.0/double(shutterSpeed);
	uniform_real_distribution<double> distribution4(-interval_time/2,interval_time/2);
	//End MB

	Scene* scene = Scene::getInstance();
	unsigned int NbLights = scene->getAreaLights().size();

	vector<Vec3Df> colorsPerLight[screenHeight][screenWidth];
	vector<vector<vector<float> > > visibilityMatrix;
	visibilityMatrix.resize(screenHeight);
	for(unsigned int k = 0 ; k<screenHeight ; k++)
	{
		visibilityMatrix[k].resize(screenWidth);
		for(unsigned int m=0; m<screenWidth; m++)
		{

			visibilityMatrix[k][m].resize(NbLights);
			colorsPerLight[k][m].resize(NbLights);
			for(unsigned l=0; l<NbLights; l++)
			{
				visibilityMatrix[k][m][l]=1.0f;
				colorsPerLight[k][m][l]=Vec3Df(0.0f, 0.0f, 0.0f);
			}
		}
	}

	Vec3Df finalColor=Vec3Df(0.0f, 0.0f, 0.0f);

	//#pragma omp parallel for schedule(dynamic, 1, private(c))
	for (unsigned int i = 0; i < screenWidth; i++) {
		progressDialog.setValue ((100*i)/screenWidth);
		for (unsigned int j = 0; j < screenHeight; j++) {
			float tanX = tan (fieldOfView) * aspectRatio;
			float tanY = tan (fieldOfView);
			float stepPixelX = float (i) - screenWidth / 2.f;
			float stepPixelY = float (j) - screenHeight / 2.f;
			vector<Vec3Df>& c = colorsPerLight[j][i];
			vector<float>& visibility = visibilityMatrix[j][i];
			//float visibility = 1.0f;
			finalColor=Vec3Df(0.0f, 0.0f, 0.0f);
			for (unsigned int l = 0 ; l < raysPerPixel ; l++) {
				for (unsigned int m = 0 ; m < raysPerPixel ; m++) {
					//#pragma omp parallel private(l, m)
					float stepAAX=0,stepAAY=0;
					if(antialiasingMode == Stochastic) {
						stepAAX = distribution3(generator) ;  
						stepAAY = distribution3(generator) ;  
					}
					else if (antialiasingMode == Uniform)
					{

						stepAAX = (float (l)) / raysPerPixel - 0.5f;  
						stepAAY = (float (m)) / raysPerPixel - 0.5f;   
					}

					Vec3Df stepX = (stepPixelX + stepAAX) / screenWidth * tanX * rightVectorOptics;
					Vec3Df stepY = (stepPixelY + stepAAY) / screenHeight * tanY * upVectorOptics;
					Vec3Df step = stepX + stepY;
					Vec3Df dir = directionOptics + step;
					vector<Vec3Df> c_optics;
					c_optics.resize(NbLights);
					for(unsigned k=0; k<NbLights; k++)
						c_optics[k]=Vec3Df(0.0f, 0.0f, 0.0f);


					if (ptMode == PTDisabled)
					{
						if (dofMode == DOFEnabled)
						{
							vector<float> v;
							for(unsigned k=0; k<NbLights; k++)
								v[k]=0.0f;

							for(unsigned int s = 0 ; s < focusBlurSamples ; s++) 
							{
								float rnd1 = distribution1(generator),rnd2 = distribution2(generator);
								Vec3Df deviationLensX = stepX,deviationLensY = stepY,deviationLens;
								deviationLensX.normalize();
								deviationLensY.normalize();
								deviationLens = deviationLensX * rnd2*cos(rnd1) + deviationLensY * rnd2*sin(rnd1);
								Vec3Df lensOrigin = camPos + deviationLens;
								Vec3Df imagePoint = camPos + dir;
								Vec3Df dirSample = imagePoint - lensOrigin;
								dirSample.normalize();

								vector<Vec3Df> c_motion;
								c_motion.resize(NbLights);
								vector<Vec3Df> c_motion_temp;
								c_motion_temp.resize(NbLights);
								for(unsigned k=0; k<NbLights; k++)
								{
									c_motion[k]=Vec3Df(0.0f, 0.0f, 0.0f);
									c_motion_temp[k]=Vec3Df(0.0f, 0.0f, 0.0f);
								}

								if(mbMode == MBEnabled) { 
									for (unsigned int t = 0 ; t < motionBlurSamples ; t++) {
										current_time = distribution4(generator);
										rayTrace(lensOrigin, dirSample,current_time, visibility, c_motion_temp);
										for(unsigned k=0; k<NbLights; k++)
										{
											c_motion[k] += c_motion_temp[k];
											v[k]=visibility[k]/float(motionBlurSamples);
										}
									}

									for(unsigned k=0; k <NbLights; k++)
									{
										c_motion[k] = c_motion[k] /(float(motionBlurSamples)*aperture); //on divise par l'ouverture pour simuler l'exposition				
										c_optics[k]+=c_motion[k];
									}
								}
								else
								{
									rayTrace(lensOrigin, dirSample, 0, visibility, c_optics);
									for(unsigned k=0; k<NbLights; k++)
										v[k]+=visibility[k];
								}
							}

							for(unsigned k=0; k<NbLights; k++)
							{
								c_optics[k] = c_optics[k] / float(focusBlurSamples);
								c[k]+= c_optics[k];
								visibilityMatrix[j][i][k] += v[k]/focusBlurSamples; 
							}
						}
						else if(mbMode == MBEnabled) 
						{
							vector<Vec3Df> c_optics_temp;
							c_optics_temp.resize(NbLights);
							for (unsigned int t = 0 ; t < motionBlurSamples ; t++) {
								current_time = distribution4(generator);
								rayTrace(camPos, dir,current_time, visibility, c_optics_temp);
								for(unsigned k=0; k<NbLights; k++)
								{
									c_optics[k]+=c_optics_temp[k];
									visibilityMatrix[j][i][k] += visibility[k]/float(motionBlurSamples);
								}

							}

							for(unsigned k=0; k<NbLights; k++)
							{
								c_optics[k] = c_optics[k] /(float(motionBlurSamples)*aperture); //on divise par l'ouverture pour simuler l'exposition
								c[k]+=c_optics[k];
							}
						} 
						else
						{
							rayTrace(camPos, dir, 0, visibility, c);
							for(unsigned k=0; k<NbLights; k++)
								visibilityMatrix[j][i][k] += visibility[k];
						}
					}
					else
					{
						finalColor += pathTrace(camPos, dir, 0, false, 10, 1.f);
					}
				}
			}


			for(unsigned k=0; k<NbLights; k++)
				visibilityMatrix[j][i][k]/=(raysPerPixel * raysPerPixel);

			finalColor *= 255.0f / (raysPerPixel * raysPerPixel);
			image.setPixel(i, j, qRgb(clamp(finalColor[0], 0, 255), clamp(finalColor[1], 0, 255), clamp(finalColor[2], 0, 255)));
		}
	}
	//The size of the filter must be an odd number
	if (ptMode == PTDisabled)
	{
		if(gaussianFilterMode != GaussianFilterDisabled)
		{
			gaussianFilter(visibilityMatrix, standardDeviation, sizeMask, screenWidth, screenHeight);
		}
		Vec3Df colorAfterFilter;

		for(unsigned int i = 0 ; i<screenWidth ; i++)
		{
			for(unsigned int j = 0 ; j<screenHeight ; j++)
			{
				colorAfterFilter=Vec3Df(0.0f, 0.0f, 0.0f);
				for (unsigned int k = 0 ; k < NbLights ; k++) 
				{
					colorAfterFilter[0] += visibilityMatrix[j][i][k]*colorsPerLight[j][i][k][0];
					colorAfterFilter[1] += visibilityMatrix[j][i][k]*colorsPerLight[j][i][k][1];
					colorAfterFilter[2] += visibilityMatrix[j][i][k]*colorsPerLight[j][i][k][2];
				}

				colorAfterFilter *= 255.0f / (raysPerPixel * raysPerPixel);
				image.setPixel(i, j, qRgb(clamp(colorAfterFilter[0], 0, 255), clamp(colorAfterFilter[1], 0, 255), clamp(colorAfterFilter[2], 0, 255)));
			}
		}
	}

	progressDialog.setValue (100);
	return image;
}

// compute the color (unoccluded) of a vertex using phong shading, the visibility of this vertex (shadows) and the occlusion rate of this vertex
void RayTracer::computeColor(const Vertex& intersectedVertex, const Object& o, const Vec3Df& dir, vector<Vec3Df>& color, vector<float>& visibility, float& occlusionRate)
{
	Scene* scene = Scene::getInstance();

	const Material& material = o.getMaterial();
	// intersected point in the object reference : intersectedPoint
	// intersected point in the world reference : intersectedPoint + o.getTrans()
	// let's put the intersected point in the world reference
	const Vec3Df& intersectedPoint = intersectedVertex.getPos() + o.getTrans();
	Vec3Df normal = intersectedVertex.getNormal();
	normal.normalize();

	if (ambientOcclusionMode != AODisabled) 
		occlusionRate=computeOcclusionRate(intersectedPoint, normal);

	if (ambientOcclusionMode != AOOnly) {
		computeShadowVisibility(intersectedPoint, o, visibility);
	}

	vector<AreaLight>& sceneLights = scene->getAreaLights();
	for (unsigned int k = 0 ; k < sceneLights.size() ; k++) {
		Vec3Df lightDirection = sceneLights[k].getPos() - intersectedPoint;
		lightDirection.normalize();
		color[k] = sceneLights[k].getIntensity() * o.getMaterial().computeColor(normal, -lightDirection, sceneLights[k].getColor(), -dir);
	}
}

float RayTracer::computeOcclusionRate(const Vec3Df& intersectedPoint, const Vec3Df& normal)
{
	Scene* scene = Scene::getInstance();
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
	return intensityAO * (1.f - ((float) occlusions) / raysAO);

}

void RayTracer::computeShadowVisibility(const Vec3Df& intersectedPoint, const Object& o, vector<float>& visibility)
{

	Scene* scene = Scene::getInstance();
	float distShadow;
	Vertex intersectionShadow;
	unsigned int leafIdShadow;
	vector<AreaLight>& sceneLights = scene->getAreaLights();

	for(unsigned k=0; k<sceneLights.size(); k++)
		visibility[k] = (float) nbPointsDisc;

	for (unsigned int k = 0 ; k < sceneLights.size() ; k++) {
		Vec3Df lightDirection = sceneLights[k].getPos() - intersectedPoint;
		float distanceToLight;
		distanceToLight = Vec3Df::distance(sceneLights[k].getPos(), intersectedPoint);
		lightDirection.normalize();

		// shadows
		// visibility is between O and 1, 0 if not visible (except for transparent objects), 1 if completely visible

		if (shadowsMode == Hard) {
			distShadow = INFINITE_DISTANCE;
			for (unsigned int n = 0 ; n < scene->getObjects().size() ; n++) {
				const Object& ob = scene->getObjects()[n];
				Ray shadowRay(intersectedPoint - ob.getTrans(), lightDirection);
				if (ob.intersectsRay(shadowRay, intersectionShadow, distShadow, leafIdShadow))
				{
					float dist = Vec3Df::distance(intersectedPoint, intersectionShadow.getPos()+ob.getTrans());
					if(dist<distanceToLight)
					{
						if(ob.getMaterial().getTransparency()<0.0001f)
						{
							visibility[k]=0.0f;
							break;
						}

						//if the object is transparent, the visibility will be equals to its transparency
						else if(visibility[k]>0.00001f)
							visibility[k]=ob.getMaterial().getTransparency()*nbPointsDisc;
					}
				}
			}
		}
		else if (shadowsMode == Soft) {
			// random set of points on the area light source
			sceneLights[k].discretize(nbPointsDisc);//, lightDirection);
			const vector<Vec3Df>& discretization = sceneLights[k].getDiscretization();

			// cast one ray for each discret point of the area light source
			for(unsigned p=0; p<nbPointsDisc; p++)
			{
				distShadow = INFINITE_DISTANCE;
				Vec3Df lightPosDisc=discretization[p];
				Vec3Df directionToLightDisc = lightPosDisc - (intersectedPoint+o.getTrans());
				directionToLightDisc.normalize();

				float v=visibility[k];
				for (unsigned int n = 0; n < scene->getObjects().size (); n++) 
				{
					const Object& ob = scene->getObjects()[n];
					Ray shadowRay(intersectedPoint  - ob.getTrans(), directionToLightDisc);
					if (ob.intersectsRay(shadowRay, intersectionShadow, distShadow, leafIdShadow)) 
					{
						float dist = Vec3Df::distance(intersectedPoint, intersectionShadow.getPos()+ob.getTrans());
						if(dist<distanceToLight)
						{
							if(ob.getMaterial().getTransparency()<0.0001f)
							{
								visibility[k]=v-1.0f;
								break;
							}
							else
								visibility[k]-=(1-ob.getMaterial().getTransparency());
						}
					}
				}
			}
		}
	}
	for(unsigned k=0; k<sceneLights.size(); k++)
		visibility[k] /= (float) nbPointsDisc;
}

void RayTracer::gaussianFilter(vector<vector<vector<float> > >& visibility, const float SIGMA, const unsigned int sizeMask, unsigned int screenWidth, unsigned int screenHeight)
{
	const float VAR = SIGMA*SIGMA;
	const float PI = 3.141592;
	const unsigned int coeffMaskHeight = screenHeight+2*(sizeMask/2);
	const unsigned int coeffMaskWidth = screenWidth+2*(sizeMask/2);
	double coeffMask [sizeMask][sizeMask];
	Scene* scene = Scene::getInstance();
	unsigned NbLights=scene->getAreaLights().size();
	float visibilityPadding [coeffMaskHeight][coeffMaskWidth][NbLights];

	float sumCoeff = 0.0f;
	//Coeff for the gaussian filter sizeMask*sizeMask
	for(unsigned int i = 0 ; i< sizeMask ; i++)
	{
		for(unsigned int j = 0 ; j< sizeMask ; j++)
		{
			//coeffMask[i][j]=0.0f;
			float k = i-sizeMask/2;
			float l = j-sizeMask/2;
			coeffMask[i][j] = exp(-(k*k + l*l)/(2*VAR))/(sqrt(2*PI)*SIGMA);
			sumCoeff+=coeffMask[i][j];
		}
	}
	//coeffMask[sizeMask/2][sizeMask/2]=1.0;

/*	//Matrix initialisation
	for(unsigned k=0; k<NbLights; k++)
	{
		for(unsigned int i = 0 ; i< coeffMaskHeight ; i++)
		{	
			if(i<sizeMask/2)
			{
				for(unsigned int j = 0 ; j<screenWidth ; j++)
					visibilityPadding[i][j+sizeMask/2-1][k]=visibility[0][j][k];

				for(unsigned int j =0; j<sizeMask/2-1; j++)
					visibilityPadding[i][j][k]=visibility[0][0][k];
				for(unsigned int j = screenWidth; j<coeffMaskWidth; j++)
					visibilityPadding[i][j][k]=visibility[0][screenWidth-1][k];
			}

			else if(i>screenHeight+sizeMask/2-1)
			{
				for(unsigned int j = 0 ; j<screenWidth ; j++)
					visibilityPadding[i][j+sizeMask/2-1][k]=visibility[screenHeight-1][j][k];
				for(unsigned int j =0; j<sizeMask/2-1; j++)
					visibilityPadding[i][j][k]=visibility[screenHeight-1][0][k];
				for(unsigned int j = screenWidth; j<coeffMaskWidth; j++)
					visibilityPadding[i][j][k]=visibility[screenHeight-1][screenWidth-1][k];
			}
			else
			{
				for(unsigned int j = 0 ; j< coeffMaskWidth ; j++)
				{	
					if(j<sizeMask/2)
						visibilityPadding[i][j][k]=visibility[i-sizeMask/2][0][k];
					if(j>sizeMask/2+screenWidth-1)
						visibilityPadding[i][j][k]=visibility[i-sizeMask/2][screenWidth-1][k];
					else
						visibilityPadding[i][j][k]=visibility[i-sizeMask/2][j-sizeMask/2][k];
				}
			}

		}
	}
*/
	for(unsigned int i = 0 ; i< coeffMaskHeight ; i++)
	{	
		for(unsigned int j = 0 ; j<coeffMaskWidth ; j++)
		{
			for(unsigned m=0;m<NbLights; m++)
			{
			if(i < sizeMask/2 || j<sizeMask/2 || i>(screenHeight-1) || j>(screenWidth-1))
			{
				visibilityPadding[i][j][m] = 0.0f;
			}
			else
			{
				visibilityPadding[i][j][m] = visibility[i-sizeMask/2][j-sizeMask/2][m];
			}
			}
		}	
	}

	cout << "padding ok" << endl;
	int halfSizeMask = sizeMask/2;
	for(unsigned m=0;m<NbLights; m++)
	{
		for(unsigned int i = 0 ; i<screenHeight ; i++)
		{
			for(unsigned int j = 0 ; j<screenWidth ; j++)
			{
				visibility[i][j][m] = 0.0f;
				for(int k = -halfSizeMask ; k <=halfSizeMask ; k++)
				{
					for(int l = -halfSizeMask ; l <= halfSizeMask ; l++)
					{
						visibility[i][j][m] += visibilityPadding[i+k+sizeMask/2][j+l+sizeMask/2][m]*coeffMask[k+sizeMask/2][l+sizeMask/2];
					}
				}
				visibility[i][j][m] /= sumCoeff;
			}	
		}
	}
}

