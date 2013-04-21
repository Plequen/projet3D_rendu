#include "BRDF.h"

using namespace std;

Vec3Df phong(const Vec3Df& point, const Vec3Df& normal, const Object& object,const Scene* scene, const Vec3Df& directionEmission, const Vec3Df& camPos)
{
	Vec3Df c(0.0f,0.0f,0.0f);
	const vector<Light>& sceneLights =  scene->getLights();
	const Material& material = object.getMaterial();

	for(unsigned l=0; l < sceneLights.size(); l++)
	{
		/****   Phong Shading with shadows ****/

		Vec3Df lightDirection = camPos - (point+object.getTrans());
		lightDirection.normalize();
		float diff = Vec3Df::dotProduct(normal, lightDirection);
		if(diff<=0.0f)
			diff=0.0f;
		Vec3Df r = 2*diff*normal-lightDirection;
		r.normalize();
		float spec = Vec3Df::dotProduct(r, directionEmission); 
		if(spec <= 0.0f)
			spec=0.0f;
		//Shadows
		//Ray shadowRay(point+object.getTrans(), Vec3Df(0.0f,0.0f,0.0f));
		//float intensity = shadowRay.computeShadowRays(point, normal, scene->getObjects(), sceneLights[l]);
		//c += intensity*(material.getDiffuse()*diff + material.getSpecular()*spec)*sceneLights[l].getColor()*material.getColor();
		c += sceneLights[l].getIntensity()*(material.getDiffuse()*diff + material.getSpecular()*pow(spec, material.getShininess()))*sceneLights[l].getColor()*material.getColor();
	}

	return c;
}

