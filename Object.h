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
	Object (const Mesh & mesh, const Material & mat);
    virtual ~Object () {}

    inline const Vec3Df & getTrans () const { return trans;}
    inline void setTrans (const Vec3Df & t) { trans = t; }

    inline const Mesh & getMesh () const { return mesh; }
    inline Mesh & getMesh () { return mesh; }
    
    inline const KDTree & getKDTree () const { return kdtree; }
    inline KDTree & getKDTree () { return kdtree; }
    inline const KDTree2& getKDTree2 () const { return kdTree2; }
    inline KDTree2& getKDTree2 () { return kdTree2; }

    inline const Material & getMaterial () const { return mat; }
    inline Material & getMaterial () { return mat; }

    inline const BoundingBox & getBoundingBox () const { return bbox; }
    void updateBoundingBox ();

	bool intersectsRay(Ray& ray, Vertex& intersectionPoint, unsigned int& leafId) const;
    
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
