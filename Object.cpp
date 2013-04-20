// *********************************************************
// Object Class
// Author : Tamy Boubekeur (boubek@gmail.com).
// Copyright (C) 2010 Tamy Boubekeur.
// All rights reserved.
// *********************************************************

#include "Object.h"

using namespace std;

Object::Object (const Mesh & mesh, const Material & mat) : mesh (mesh), mat (mat) {
        updateBoundingBox ();
	int inter = 1;
	std::cout << "building kdtree" << std::endl;
	clock_t begin = clock();
	for (int i = 0 ; i < inter ; i++)
		kdtree.buildKDTree(mesh);
	clock_t end = clock();
	double sec1 = (double) ( end-begin ) / CLOCKS_PER_SEC;
	cout << "KDTree build v1 : " << sec1 << "sec" << endl;
//	kdtree.printTree();
	std::cout << "kdtree built" << std::endl;
	std::cout << "building kdtree2" << std::endl;
	begin = clock();
	for (int i = 0 ; i < inter ; i++)
		kdTree2.buildKDTree(mesh);
	end = clock();
	double sec2 = (double) ( end-begin ) / CLOCKS_PER_SEC;
	cout << "KDTree build v2 : " << sec2 << "sec" << endl;

	std::cout << "kdtree2 built" << std::endl << std::endl;
}
bool Object::intersectsRay(Ray& ray, Vertex& intersectionPoint, float& t, unsigned int& leafId) const {
	return kdTree2.intersectsRay(mesh, ray, intersectionPoint, t, leafId);
}

void Object::updateBoundingBox () {
    const vector<Vertex> & V = mesh.getVertices ();
    if (V.empty ())
        bbox = BoundingBox ();
    else {
        bbox = BoundingBox (V[0].getPos ());
        for (unsigned int i = 1; i < V.size (); i++)
            bbox.extendTo (V[i].getPos ());
    }
}
