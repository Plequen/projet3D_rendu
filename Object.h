// *********************************************************
// Object Class
// Author : Tamy Boubekeur (boubek@gmail.com).
// Copyright (C) 2010 Tamy Boubekeur.
// All rights reserved.
// *********************************************************

#ifndef OBJECT_H
#define OBJECT_H

#include <iostream>
#include <vector>
#include <ctime>

#include "Mesh.h"
#include "KDTree.h"
#include "KDTree2.h"
#include "Material.h"
#include "BoundingBox.h"

class Object {
public:
    inline Object () {}
    inline Object (const Mesh & mesh, const Material & mat) : mesh (mesh), mat (mat) {
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
		kdTree2.buildKDTree(&(this->mesh));
	end = clock();
	double sec2 = (double) ( end-begin ) / CLOCKS_PER_SEC;
	cout << "KDTree build v2 : " << sec2 << "sec" << endl;

	std::cout << "kdtree2 built" << std::endl << std::endl;
    }
    virtual ~Object () {}

    inline const Vec3Df & getTrans () const { return trans;}
    inline void setTrans (const Vec3Df & t) { trans = t; }

    inline const Mesh & getMesh () const { return mesh; }
    inline Mesh & getMesh () { return mesh; }
    
    inline const KDTree & getKDTree () const { return kdtree; }
    inline KDTree & getKDTree () { return kdtree; }

    inline const Material & getMaterial () const { return mat; }
    inline Material & getMaterial () { return mat; }

    inline const BoundingBox & getBoundingBox () const { return bbox; }
    void updateBoundingBox ();
    
private:
    Mesh mesh;
    KDTree kdtree;
	KDTree2 kdTree2;
    Material mat;
    BoundingBox bbox;
    Vec3Df trans;
};


#endif // Scene_H

// Some Emacs-Hints -- please don't remove:
//
//  Local Variables:
//  mode:C++
//  tab-width:4
//  End:
