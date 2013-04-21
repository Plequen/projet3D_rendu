
#include "AreaLight.h"

void AreaLight::discretize(unsigned k)
{
	if(discretization.size()!=k)
		discretization.resize(k);	
	Vec3Df x1(0.0f, 0.0f, 0.0f);
	Vec3Df y1;
	if(orientation[0]<=0.00001f && orientation[0] >=-0.00001f)
		x1=Vec3Df(0.0f,-orientation[2], orientation[1]);
	else
		x1=Vec3Df(-orientation[2]/orientation[1], 1.0f, 0.0f);

	y1=Vec3Df::crossProduct(x1, -orientation);
	for(unsigned i=0; i<k; i++)
	{
		float r=rayon*rand()/RAND_MAX;
		float angle=2*3.14*rand()/RAND_MAX;
		Vec3Df v(r*cos(angle), r*sin(angle), 0.0f);
		Vec3Df v2(x1[0]*v[0]+y1[0]*v[1],x1[1]*v[0]+y1[1]*v[1],x1[2]*v[0]+y1[2]*v[1]);
		discretization[i]=pos+v2;
	}
}
 
