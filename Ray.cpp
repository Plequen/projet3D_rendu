// *********************************************************
// Ray Class
// Author : Tamy Boubekeur (boubek@gmail.com).
// Copyright (C) 2010 Tamy Boubekeur.
// All rights reserved.
// *********************************************************

#include "Ray.h"

using namespace std;

static const unsigned int NUMDIM = 3, RIGHT = 0, LEFT = 1, MIDDLE = 2;

bool Ray::intersect (const BoundingBox & bbox, float & tmin) const {
	const Vec3Df & minBb = bbox.getMin ();
	const Vec3Df & maxBb = bbox.getMax ();
	bool inside = true;
	unsigned int  quadrant[NUMDIM];
	register unsigned int i;
	unsigned int whichPlane;
	Vec3Df maxT;
	Vec3Df candidatePlane;
	Vec3Df intersectionPoint;

	for (i=0; i<NUMDIM; i++)
		if (origin[i] < minBb[i]) {
			quadrant[i] = LEFT;
			candidatePlane[i] = minBb[i];
			inside = false;
		} else if (origin[i] > maxBb[i]) {
			quadrant[i] = RIGHT;
			candidatePlane[i] = maxBb[i];
			inside = false;
		} else	{
			quadrant[i] = MIDDLE;
		}

	if (inside)	{
		intersectionPoint = origin;
		return (true);
	}

	for (i = 0; i < NUMDIM; i++)
		if (quadrant[i] != MIDDLE && direction[i] !=0.)
			maxT[i] = (candidatePlane[i]-origin[i]) / direction[i];
		else
			maxT[i] = -1.;

	whichPlane = 0;
	for (i = 1; i < NUMDIM; i++)
		if (maxT[whichPlane] < maxT[i])
			whichPlane = i;

	if (maxT[whichPlane] < 0.) return (false);
	for (i = 0; i < NUMDIM; i++)
		if (whichPlane != i) {
			intersectionPoint[i] = origin[i] + maxT[whichPlane] *direction[i];
			if (intersectionPoint[i] < minBb[i] || intersectionPoint[i] > maxBb[i])
				return (false);
		} else {
			intersectionPoint[i] = candidatePlane[i];
		}
	tmin=maxT[whichPlane];
	return (true);			
}
bool Ray::intersect (const BoundingBox & bbox, Vec3Df & intersectionPoint) const {
	const Vec3Df & minBb = bbox.getMin ();
	const Vec3Df & maxBb = bbox.getMax ();
	bool inside = true;
	unsigned int  quadrant[NUMDIM];
	register unsigned int i;
	unsigned int whichPlane;
	Vec3Df maxT;
	Vec3Df candidatePlane;

	for (i=0; i<NUMDIM; i++)
		if (origin[i] < minBb[i]) {
			quadrant[i] = LEFT;
			candidatePlane[i] = minBb[i];
			inside = false;
		} else if (origin[i] > maxBb[i]) {
			quadrant[i] = RIGHT;
			candidatePlane[i] = maxBb[i];
			inside = false;
		} else	{
			quadrant[i] = MIDDLE;
		}

	if (inside)	{
		intersectionPoint = origin;
		return (true);
	}

	for (i = 0; i < NUMDIM; i++)
		if (quadrant[i] != MIDDLE && direction[i] !=0.)
			maxT[i] = (candidatePlane[i]-origin[i]) / direction[i];
		else
			maxT[i] = -1.;

	whichPlane = 0;
	for (i = 1; i < NUMDIM; i++)
		if (maxT[whichPlane] < maxT[i])
			whichPlane = i;

	if (maxT[whichPlane] < 0.) return (false);
	for (i = 0; i < NUMDIM; i++)
		if (whichPlane != i) {
			intersectionPoint[i] = origin[i] + maxT[whichPlane] *direction[i];
			if (intersectionPoint[i] < minBb[i] || intersectionPoint[i] > maxBb[i])
				return (false);
		} else {
			intersectionPoint[i] = candidatePlane[i];
		}
	return (true);			
}

/*bool Ray::intersectObject(const Object & o, Vec3Df & intersectionPoint, Vec3Df & normal) const
{
	bool intersection=false;
	const Mesh & m = o.getMesh();
	const KDTree & kdtree = o.getKDTree();
	Node* node = kdtree.getRoot();
	stack<Node*> stackNode;
	const std::vector<Triangle> & triangles = m.getTriangles();
	const std::vector<Vertex> & vertices = m.getVertices();
	float t;
	float tmin = 10000.0f;
	float coef1, coef2;
	float coefBary1, coefBary2;
	unsigned tri;

	bool end = false;
	while(!end)
	{
		if(node->isLeaf())
		{
			const unsigned* trianglesLeaf = node->getTriangles();
			for(unsigned i=0; i<node->getNbTriangles(); i++)
			{	
				const Vec3Df & va = vertices[triangles[trianglesLeaf[i]].getVertex(0)].getPos();
				const Vec3Df & vb = vertices[triangles[trianglesLeaf[i]].getVertex(1)].getPos();
				const Vec3Df & vc = vertices[triangles[trianglesLeaf[i]].getVertex(2)].getPos();
				if(intersectTriangle(va, vb, vc, t, coef1, coef2))
				{
					intersection=true;
					end=true;
					if(t < tmin)
					{
						tmin=t;
						coefBary1=coef1;
						coefBary2=coef2;
						tri=trianglesLeaf[i];
					}
				}
			}
			if(!intersection)
			{
				if(stackNode.size()==0)
					end=true;
				else
				{	
					node=stackNode.top();
					stackNode.pop();
				}
			}
		} 

		else
		{
			float t1, t2;
			bool b1 = false;
			bool b2 = false;
			b1 = intersect((node->getLeftChild())->getBoundingBox(), t1);
			b2 = intersect((node->getRightChild())->getBoundingBox(), t2);

			if(b1 && b2)
			{
				if(t1<t2)
				{
					stackNode.push(node->getRightChild());
					node=node->getLeftChild();
				}
				else
				{
					stackNode.push(node->getLeftChild());
					node=node->getRightChild();
				}
			}
			else if(b1 && !b2)
				node=node->getLeftChild();
			else if(!b1 && b2)
				node=node->getRightChild();
			else
				end=true;
		}	 
	}

	if(intersection)
	{
		intersectionPoint = origin + tmin*direction;
		normal=(1-coefBary1-coefBary2)*vertices[triangles[tri].getVertex(0)].getNormal()
			+ coefBary1*vertices[triangles[tri].getVertex(1)].getNormal()
			+ coefBary2*vertices[triangles[tri].getVertex(2)].getNormal();
	}

	return intersection;
}*/

bool Ray::intersectTriangle(const Vec3Df& va, const Vec3Df & vb, const Vec3Df & vc, float & t, float & coef1, float & coef2) const
{
	float M;
	Vec3Df v1 = va-vb;
	Vec3Df v2 = va-vc;
	Vec3Df v3 = va-origin;

	M = v1[0]*(v2[1]*direction[2]-direction[1]*v2[2])+v1[1]*(direction[0]*v2[2]-v2[0]*direction[2])+v1[2]*(v2[0]*direction[1]-v2[1]*direction[0]);
	coef1 = (v3[0]*(v2[1]*direction[2]-direction[1]*v2[2])+v3[1]*(direction[0]*v2[2]-v2[0]*direction[2])+v3[2]*(v2[0]*direction[1]-v2[1]*direction[0]))/M;

	if(coef1 < 0 || coef1 > 1)
		return false; 

	coef2 = (direction[2]*(v1[0]*v3[1]-v3[0]*v1[1])+direction[1]*(v3[0]*v1[2]-v1[0]*v3[2])+direction[0]*(v1[1]*v3[2]-v3[1]*v1[2]))/M;

	if(coef2 < 0 || coef1+coef2 > 1)
		return false;

	t = -(v2[2]*(v1[0]*v3[1]-v3[0]*v1[1])+v2[1]*(v3[0]*v1[2]-v1[0]*v3[2])+v2[0]*(v1[1]*v3[2]-v3[1]*v1[2]))/M;
	if(t <= 0.0001f)
		return false;

	return true; 

}
