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
	QProgressDialog progressDialog ("Raytracing...", "Cancel", 0, 100);
	progressDialog.show ();
	for (unsigned int i = 0; i < screenWidth; i++) 
	{
		progressDialog.setValue ((100*i)/screenWidth);
		for (unsigned int j = 0; j < screenHeight; j++) 
		{
			//Paramètres variables
			bool softShadows=true;
			bool hardShadows=false;
			unsigned nbRaysPerPixel=2; // 2 => distribution 2*2, 3 => distribution 3*3, etc 
			unsigned nbPointsDisc = 10; // nombre de point répartis aléatoirement sur la source étendue

			float tanX = tan (fieldOfView)*aspectRatio;
			float tanY = tan (fieldOfView);
			float pixelWidth =tanX/screenWidth;
			float pixelHeight = tanY/screenHeight;
			Vec3Df stepX = (float (i) - screenWidth/2.f) * pixelWidth * rightVector;
			Vec3Df stepY = (float (j) - screenHeight/2.f) * pixelHeight * upVector;
			Vec3Df step = stepX + stepY;
			Vec3Df dir = direction + step;
			dir.normalize ();
			Vertex intersectionPoint;
			unsigned objectIntersectedIndex; // retient l'objet de la scene qui a été intersecté
			float smallestIntersectionDistance = 1000000.f;
			Vec3Df c (backgroundColor);
			bool hasIntersection=false;  
			vector<Vec3Df> miniSteps;  // Pour créer des points espacés régulirement à l'intérieur du pixel
			miniSteps.resize(nbRaysPerPixel*nbRaysPerPixel);
			vector<Vec3Df> colors; // c sera la moyenne des couleurs obtenu pour chaque rayon du pixel
			colors.resize(nbRaysPerPixel*nbRaysPerPixel);
			for(unsigned i1=0; i1<colors.size(); i1++)
				colors[i1]=Vec3Df(backgroundColor);

			// On crée des points espacés régulièrement à l'intérieur du pixel
			for(unsigned rx=0; rx<nbRaysPerPixel; rx++)
			{
				for(unsigned ry=0; ry<nbRaysPerPixel; ry++)
				{	
					miniSteps[ry*nbRaysPerPixel+rx] = Vec3Df(((float)(rx+1)/(float)(nbRaysPerPixel+1)-0.5f)*pixelWidth,
							((float)(ry+1)/(float)(nbRaysPerPixel+1)-0.5f)*pixelHeight,0);
				}
			}

			//On cherche l'intersection de chacun des rayons passant par un point du pixel avec la scene
			for(unsigned r=0; r<nbRaysPerPixel*nbRaysPerPixel; r++)
			{
				//On test tous les objets de la scene et on ne garde que l'intersection de l'objet le plus proche
				for (unsigned int k = 0; k < scene->getObjects().size (); k++) 
				{
					Vertex intersectionPointTemp;
					const Object & o = scene->getObjects()[k];
					Ray ray(camPos-o.getTrans (), dir+miniSteps[r]);
					if (ray.intersectObject (o, intersectionPointTemp))
					{	
						float intersectionDistance = Vec3Df::squaredDistance (intersectionPointTemp.getPos() + o.getTrans (), camPos);
						if (intersectionDistance < smallestIntersectionDistance) 
						{
							hasIntersection=true;
							objectIntersectedIndex=k;
							intersectionPoint=intersectionPointTemp;
							smallestIntersectionDistance = intersectionDistance;
						}
					}
				}

				//Si le rayon a intersecté un triangle
				if(hasIntersection)
				{
					//L'objet sera noir s'il n'est visible par aucune source lumineuse
					colors[r] = Vec3Df(0.0f,0.0f,0.0f);
					const Object & o = scene->getObjects()[objectIntersectedIndex];
					Material material = o.getMaterial();
					std::vector<AreaLight> sceneAreaLights = scene->getAreaLights();

					//On traite chaque source de lumière
					for(unsigned l=0; l < sceneAreaLights.size(); l++)
					{
						// Position du point en World Space
						Vec3Df pointWS = intersectionPoint.getPos()+o.getTrans();

						// Position de la source lumineuse, elle est fixe dans l'espace caméra
						Vec3Df lightPos=sceneAreaLights[l].getPos();
						lightPos[0]=Vec3Df::dotProduct(rightVector,lightPos);
						lightPos[1]=Vec3Df::dotProduct(upVector,lightPos);
						lightPos[2]=Vec3Df::dotProduct(-direction,lightPos);
						lightPos+=camPos;

						//Direction du point vers la source lumineuse
						Vec3Df directionToLight = lightPos - pointWS;
						directionToLight.normalize();
						float visibility = (float)nbPointsDisc;

						//Si l'on veut représenter des ombres douces
						if(softShadows)
						{
							// On répartit aléatoirement des point sur la surface de la source
							sceneAreaLights[l].discretize(nbPointsDisc);
							const vector<Vec3Df>& discretization = sceneAreaLights[l].getDiscretization();

							// Pour chacun des points discrétisés On va lancé un rayon vers chacun des points discrétisé
							for(unsigned n=0; n<nbPointsDisc; n++)
							{
								// Le point discrétisé de la source étendue reste fixe par rapport à la caméra
								Vec3Df lightPosDisc=discretization[n];
								lightPosDisc[0]=Vec3Df::dotProduct(rightVector,lightPosDisc);
								lightPosDisc[1]=Vec3Df::dotProduct(upVector,lightPosDisc);
								lightPosDisc[2]=Vec3Df::dotProduct(-direction,lightPosDisc);
								lightPosDisc+=camPos;

								Vec3Df directionToLightDisc = lightPosDisc - pointWS;
								directionToLightDisc.normalize();

								//On test si le point d'intersection est visible du point
								// discretisé de la source lumineuse
								for (unsigned int k = 0; k < scene->getObjects().size (); k++) 
								{
									Vertex intersectionPointTemp;
									const Object & oTemp = scene->getObjects()[k];
									Ray rayPointToLightDisc(pointWS-oTemp.getTrans(), directionToLightDisc);
									// 
									if (rayPointToLightDisc.intersectObject (oTemp, intersectionPointTemp))
									{
										//Un objet cache le point discretisé de la source étendue
										//Ce point de la source étendu n'éclaire donc pas le point d'intersection 
										visibility--;
										// Pas la peine de chercher d'autres intersections avec d'autres objets
										break;
									}
								}
							}
						}// On a fini de traiter les ombres douces

						//Si l'on veut représenter des ombres dures
						//On ne considére que des sources ponctuelles
						//On n'envoie par conséquent qu'un rayon vers la source lumineuse
						else if(hardShadows)
						{
							for (unsigned int k = 0; k < scene->getObjects().size (); k++) 
							{
								Vertex intersectionPointTemp;
								const Object & oTemp = scene->getObjects()[k];
								Ray rayPointToLight(pointWS-oTemp.getTrans(), directionToLight);
								if (rayPointToLight.intersectObject (oTemp, intersectionPointTemp))
								{
									visibility=0.0f; // L'objet n'est pas éclairé
									break;
								}
							}
						}

						visibility/=(float)nbPointsDisc;

						//Si l'objet est au moins partiellement éclairé
						if(visibility>0.0f)
						{
							//Phong Shading


							//la normal de l'objet en ce point
							//Seule des translations ont été appliquées à l'objet, la normale n'est dont pas modifiée
							Vec3Df normal=intersectionPoint.getNormal()/intersectionPoint.getNormal().getLength();

							float diff = Vec3Df::dotProduct(normal, directionToLight);
							Vec3Df reflected = 2*diff*normal-directionToLight;
							if(diff<=0.0f)
								diff=0.0f;
							reflected.normalize();
							float spec = Vec3Df::dotProduct(reflected, -dir); 
							if(spec <= 0.0f)
								spec=0.0f;

							colors[r] += sceneAreaLights[l].getIntensity()*(material.getDiffuse()*diff 
									+ material.getSpecular()*spec)*sceneAreaLights[l].getColor()*material.getColor();
							
							//l'intensité est plus ou moins forte selon que le point est plus ou moins eclairé
							colors[r]*=visibility;
						}
					}// On a fini de traiter chacune des lumières de la scène
				}// On a fini de calculer la couleur du pixel lorsqu'un rayon intersecte la scène
				c+=colors[r];
			}// On a traité tous les rayons envoyés à l'intérieur d'un même pixel

			c=255.0f*c/colors.size(); // On fait la moyenne de la couleur obtenue pour chaque rayon;
			image.setPixel (i, j, qRgb (clamp (c[0], 0, 255), clamp (c[1], 0, 255), clamp (c[2], 0, 255)));
		}
	}
	progressDialog.setValue (100);
	return image;
}
